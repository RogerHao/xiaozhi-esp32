#ifndef __SERVO_CONTROLLER_H__
#define __SERVO_CONTROLLER_H__

#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SERVO_MIN_PULSEWIDTH_US 500           // 最小脉宽（微秒）
#define SERVO_MAX_PULSEWIDTH_US 2500          // 最大脉宽（微秒）
#define SERVO_MIN_DEGREE 0                    // 最小角度
#define SERVO_MAX_DEGREE 180                  // 最大角度
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD 20000           // 20000 ticks, 20ms

/**
 * @brief 舵机控制器类
 * 
 * 用于控制数字舵机，支持角度设置、摆动等基本功能
 */
class ServoController {
public:
    /**
     * @brief 构造函数
     * @param pin 舵机控制引脚
     * @param trim 舵机微调值（度）
     */
    ServoController(int pin, int trim = 0);
    
    /**
     * @brief 析构函数
     */
    ~ServoController();
    
    /**
     * @brief 初始化舵机
     * @return ESP_OK成功，其他值失败
     */
    esp_err_t Initialize();
    
    /**
     * @brief 设置舵机角度
     * @param angle 目标角度（0-180度）
     * @return ESP_OK成功，其他值失败
     */
    esp_err_t SetAngle(int angle);
    
    /**
     * @brief 获取当前角度
     * @return 当前角度
     */
    int GetAngle() const { return current_angle_; }
    
    /**
     * @brief 设置舵机微调值
     * @param trim 微调值（度）
     */
    void SetTrim(int trim) { trim_ = trim; }
    
    /**
     * @brief 获取舵机微调值
     * @return 微调值
     */
    int GetTrim() const { return trim_; }
    
    /**
     * @brief 摆动舵机
     * @param start_angle 起始角度
     * @param end_angle 结束角度
     * @param cycles 摆动次数
     * @param period_ms 每次摆动周期（毫秒）
     * @return ESP_OK成功，其他值失败
     */
    esp_err_t Oscillate(int start_angle, int end_angle, int cycles, int period_ms);
    
    /**
     * @brief 停止舵机
     */
    void Stop();

private:
    /**
     * @brief 将角度转换为PWM占空比
     * @param angle 角度
     * @return PWM占空比值
     */
    uint32_t AngleToDuty(int angle);
    
    /**
     * @brief 写入PWM信号
     * @param duty PWM占空比
     * @return ESP_OK成功，其他值失败
     */
    esp_err_t WritePwm(uint32_t duty);

private:
    int pin_;                    // 舵机控制引脚
    int trim_;                   // 舵机微调值
    int current_angle_;          // 当前角度
    bool is_initialized_;        // 是否已初始化
    ledc_channel_t ledc_channel_; // LEDC通道
    ledc_mode_t ledc_speed_mode_; // LEDC速度模式
    bool is_oscillating_;        // 是否正在摆动
    TaskHandle_t oscillate_task_; // 摆动任务句柄
};

#endif // __SERVO_CONTROLLER_H__ 