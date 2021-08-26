#pragma once

#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "driver/gpio.h"

#include <hx711.h>

namespace scale::raw {
    struct ScaleConfig {
        gpio_num_t gpioDAT;
        gpio_num_t gpioCLK;
    };

    struct ScaleEvent {
        int rawData;
    };

    class Scale {
    public:
        Scale(const ScaleConfig &config) : _config(config) {
            _dev = {
                .dout = _config.gpioDAT,
                .pd_sck = _config.gpioCLK,
                .gain = HX711_GAIN_A_64
            };
            _scaleEventQueue = xQueueCreate(20, sizeof(ScaleEvent));
            start();
        }

        ScaleEvent getEvent();
    private:
        void start();
        std::string tag() const;

        static void process(void *arg);

        hx711_t _dev;
        xQueueHandle _scaleEventQueue;
        ScaleConfig _config;
    };
}
