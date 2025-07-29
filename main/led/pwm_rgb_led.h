#ifndef _PWM_RGB_LED_H_
#define _PWM_RGB_LED_H_

#include "led.h"
#include <driver/gpio.h>
#include <cstdint>

class PwmRgbLed {
public:
    PwmRgbLed(gpio_num_t r_pin, gpio_num_t g_pin, gpio_num_t b_pin);
    ~PwmRgbLed();

    void SetColor(uint8_t r, uint8_t g, uint8_t b);
    void TurnOn();
    void TurnOff();
    bool IsOn() const;

private:
    void Initialize();

    gpio_num_t r_pin_;
    gpio_num_t g_pin_;
    gpio_num_t b_pin_;
    bool is_on_ = false;
    uint8_t r_, g_, b_;
};

#endif // _PWM_RGB_LED_H_ 