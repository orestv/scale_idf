#pragma once

#include <string>

#include "scale/events.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/gpio.h>

namespace scale::peri::button {
    struct ButtonConfig {
        gpio_num_t buttonGPIO;
        esp_event_loop_handle_t eventLoop;
        events::Event eventID;
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
            _q = xQueueCreate(10, 0);
            start();
        }

    private:
        static void IRAM_ATTR gpio_isr_handler(void* arg);

        std::string tag() const;
        void start();

        xQueueHandle _q;

        Debouncer _debouncer;
        ButtonConfig _config;
    };
}