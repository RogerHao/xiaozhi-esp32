#include "pwm_rgb_led.h"
#include <driver/ledc.h>
#include <esp_log.h>

#define LEDC_TIMER              LEDC_TIMER_1
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_R          LEDC_CHANNEL_1
#define LEDC_CHANNEL_G          LEDC_CHANNEL_2
#define LEDC_CHANNEL_B          LEDC_CHANNEL_3
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 8 bits
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

static const char* TAG = "PwmRgbLed";

PwmRgbLed::PwmRgbLed(gpio_num_t r_pin, gpio_num_t g_pin, gpio_num_t b_pin)
    : r_pin_(r_pin), g_pin_(g_pin), b_pin_(b_pin), r_(0), g_(0), b_(0) {
    Initialize();
}

PwmRgbLed::~PwmRgbLed() {
    ledc_stop(LEDC_MODE, LEDC_CHANNEL_R, 0);
    ledc_stop(LEDC_MODE, LEDC_CHANNEL_G, 0);
    ledc_stop(LEDC_MODE, LEDC_CHANNEL_B, 0);
}

void PwmRgbLed::Initialize() {
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel_r = {
        .gpio_num   = r_pin_,
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL_R,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config_t ledc_channel_g = {
        .gpio_num   = g_pin_,
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL_G,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config_t ledc_channel_b = {
        .gpio_num   = b_pin_,
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL_B,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_r));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_g));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_b));
    ESP_LOGI(TAG, "Initialized PWM RGB LED on R:%d, G:%d, B:%d", r_pin_, g_pin_, b_pin_);
    TurnOff();
}

void PwmRgbLed::SetColor(uint8_t r, uint8_t g, uint8_t b) {
    r_ = r;
    g_ = g;
    b_ = b;
    if (is_on_) {
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R, r_));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_R));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_G, g_));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_G));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, b_));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B));
        ESP_LOGD(TAG, "Set color to R:%d, G:%d, B:%d", r_, g_, b_);
    }
}

void PwmRgbLed::TurnOn() {
    is_on_ = true;
    SetColor(r_, g_, b_);
    ESP_LOGD(TAG, "Turned on");
}

void PwmRgbLed::TurnOff() {
    is_on_ = false;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_R));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_G, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_G));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B));
    ESP_LOGD(TAG, "Turned off");
}

bool PwmRgbLed::IsOn() const {
    return is_on_;
} 