#include "servo_controller.h"
#include <cmath>

static const char* TAG = "ServoController";

// 摆动任务参数结构体
struct OscillateTaskParams {
    ServoController* servo;
    int start_angle;
    int end_angle;
    int cycles;
    int period_ms;
};

ServoController::ServoController(int pin, int trim) 
    : pin_(pin), trim_(trim), current_angle_(90), is_initialized_(false), 
      ledc_channel_(LEDC_CHANNEL_0), ledc_speed_mode_(LEDC_LOW_SPEED_MODE),
      is_oscillating_(false), oscillate_task_(nullptr) {
}

ServoController::~ServoController() {
    Stop();
    if (is_initialized_) {
        ledc_stop(ledc_speed_mode_, ledc_channel_, 0);
    }
}

esp_err_t ServoController::Initialize() {
    if (is_initialized_) {
        return ESP_OK;
    }

    // 配置LEDC定时器
    ledc_timer_config_t ledc_timer = {
        .speed_mode = ledc_speed_mode_,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 50,  // 50Hz PWM频率
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 分配LEDC通道
    static int last_channel = 0;
    last_channel = (last_channel + 1) % 8;  // 使用8个通道循环
    ledc_channel_ = (ledc_channel_t)last_channel;

    // 配置LEDC通道
    ledc_channel_config_t ledc_channel = {
        .gpio_num = pin_,
        .speed_mode = ledc_speed_mode_,
        .channel = ledc_channel_,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    is_initialized_ = true;
    ESP_LOGI(TAG, "Servo initialized on pin %d, channel %d", pin_, ledc_channel_);
    
    // 设置初始位置到中间位置
    return SetAngle(90);
}

esp_err_t ServoController::SetAngle(int angle) {
    if (!is_initialized_) {
        ESP_LOGE(TAG, "Servo not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // 限制角度范围
    angle = std::max(SERVO_MIN_DEGREE, std::min(SERVO_MAX_DEGREE, angle));
    
    // 应用微调
    int adjusted_angle = angle + trim_;
    adjusted_angle = std::max(SERVO_MIN_DEGREE, std::min(SERVO_MAX_DEGREE, adjusted_angle));
    
    current_angle_ = angle;
    
    uint32_t duty = AngleToDuty(adjusted_angle);
    esp_err_t ret = WritePwm(duty);
    
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "Set servo angle to %d (adjusted: %d)", angle, adjusted_angle);
    }
    
    return ret;
}

esp_err_t ServoController::Oscillate(int start_angle, int end_angle, int cycles, int period_ms) {
    if (!is_initialized_) {
        ESP_LOGE(TAG, "Servo not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (is_oscillating_) {
        ESP_LOGW(TAG, "Servo is already oscillating");
        return ESP_ERR_INVALID_STATE;
    }

    // 限制角度范围
    start_angle = std::max(SERVO_MIN_DEGREE, std::min(SERVO_MAX_DEGREE, start_angle));
    end_angle = std::max(SERVO_MIN_DEGREE, std::min(SERVO_MAX_DEGREE, end_angle));
    
    if (start_angle == end_angle) {
        ESP_LOGW(TAG, "Start and end angles are the same");
        return ESP_ERR_INVALID_ARG;
    }

    // 创建摆动任务参数
    OscillateTaskParams* params = new OscillateTaskParams{
        .servo = this,
        .start_angle = start_angle,
        .end_angle = end_angle,
        .cycles = cycles,
        .period_ms = period_ms
    };

    // 创建摆动任务
    BaseType_t ret = xTaskCreate(
        [](void* param) {
            OscillateTaskParams* params = static_cast<OscillateTaskParams*>(param);
            ServoController* servo = params->servo;
            
            servo->is_oscillating_ = true;
            ESP_LOGI(TAG, "Start oscillating from %d to %d, %d cycles", 
                     params->start_angle, params->end_angle, params->cycles);
            
            for (int i = 0; i < params->cycles && servo->is_oscillating_; i++) {
                // 从起始角度到结束角度
                servo->SetAngle(params->start_angle);
                vTaskDelay(pdMS_TO_TICKS(params->period_ms / 2));
                
                if (!servo->is_oscillating_) break;
                
                // 从结束角度到起始角度
                servo->SetAngle(params->end_angle);
                vTaskDelay(pdMS_TO_TICKS(params->period_ms / 2));
            }
            
            servo->is_oscillating_ = false;
            servo->oscillate_task_ = nullptr;
            ESP_LOGI(TAG, "Oscillation completed");
            
            delete params;
            vTaskDelete(nullptr);
        },
        "servo_oscillate",
        4096,
        params,
        5,
        &oscillate_task_
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create oscillation task");
        delete params;
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

void ServoController::Stop() {
    if (is_oscillating_) {
        is_oscillating_ = false;
        if (oscillate_task_) {
            vTaskDelay(pdMS_TO_TICKS(100));  // 给任务一点时间退出
            oscillate_task_ = nullptr;
        }
        ESP_LOGI(TAG, "Servo oscillation stopped");
    }
}

uint32_t ServoController::AngleToDuty(int angle) {
    // 将角度转换为脉宽（微秒）
    float pulse_width = SERVO_MIN_PULSEWIDTH_US + 
                       (angle / 180.0f) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US);
    
    // 将脉宽转换为占空比
    // 50Hz = 20ms周期，13位分辨率 = 8192
    uint32_t duty = (uint32_t)((pulse_width / 20000.0f) * 8192.0f);
    
    return duty;
}

esp_err_t ServoController::WritePwm(uint32_t duty) {
    esp_err_t ret = ledc_set_duty(ledc_speed_mode_, ledc_channel_, duty);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set duty: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = ledc_update_duty(ledc_speed_mode_, ledc_channel_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update duty: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
} 