#pragma once

#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/gpio.h>

namespace scale::peri::button {
    struct ButtonConfig {
        gpio_num_t buttonGPIO;
    };

    class Debouncer {
    public:
        Debouncer(uint32_t debounceMillis = 750): _debounceMillis(debounceMillis) {}
        bool trigger();
    private:
        uint32_t _debounceMillis;
        uint32_t _lastTriggerTime;
    };

    class PushButton {
    public:
        PushButton(const ButtonConfig &config): _config(config) {
            _debouncer = Debouncer();
            _buttonEventQueue = xQueueCreate(10, sizeof(gpio_num_t));
            start();
        }


    private:
        static void IRAM_ATTR gpio_isr_handler(void* arg);
        static void gpioTask(void *arg);

        std::string tag() const;
        void start();

        Debouncer _debouncer;
        xQueueHandle _buttonEventQueue;
        ButtonConfig _config;
    };
}