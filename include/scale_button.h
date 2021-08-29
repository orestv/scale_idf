#pragma once

#include <string>

#include "scale_events.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/gpio.h>

namespace scale::peri::button {
    struct ButtonConfig {
        gpio_num_t buttonGPIO;
        esp_event_loop_handle_t eventLoop;
    };

    struct ButtonEvent {

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
            _buttonEventQueue = xQueueCreate(10, sizeof(ButtonEvent));
            start();
        }

        ButtonEvent getEvent();

    private:
        static void IRAM_ATTR gpio_isr_handler(void* arg);

        std::string tag() const;
        void start();

        Debouncer _debouncer;
        xQueueHandle _buttonEventQueue;
        ButtonConfig _config;
    };
}