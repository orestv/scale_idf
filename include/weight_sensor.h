#pragma once

#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "driver/gpio.h"

#include <hx711.h>

namespace scale::weight {
    struct ScaleConfig {
        gpio_num_t gpioDAT;
        gpio_num_t gpioCLK;
    };

    class Scale {
    public:
        Scale(const ScaleConfig &config) : _config(config) {
            _dev = {
                .dout = _config.gpioDAT,
                .pd_sck = _config.gpioCLK,
                .gain = HX711_GAIN_A_128
            };
            start();
        }

    private:
        void start();
        std::string tag() const;

        static void process(void *arg);

        hx711_t _dev;
        ScaleConfig _config;
    };
}
