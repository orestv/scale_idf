#include "scale_button.h"

#include <sstream>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <esp_log.h>

namespace scale::peri::button {

    // const char *TAG = "Button";

    void PushButton::start() {
        ESP_LOGI(tag().c_str(), "Configuring button on GPIO %d", _config.buttonGPIO);
        gpio_config_t buttonConfig = {
            .pin_bit_mask = 1ULL << _config.buttonGPIO,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_NEGEDGE,
        };

        xTaskCreate(
            [](void *arg) {
                auto &_this = *(PushButton*)arg;
                while (true) {
                    xQueueReceive(_this._q, nullptr, portMAX_DELAY);
                    ESP_LOGD(_this.tag().c_str(), "Button receive q event");
                    auto level = gpio_get_level(_this._config.buttonGPIO);
                    ESP_LOGD(_this.tag().c_str(), "Button level: %i", level);
                    if (level == 0) {
                        ESP_LOGD(_this.tag().c_str(), "Checking debouncer");
                        if (!_this._debouncer.trigger()) {
                            ESP_LOGD(_this.tag().c_str(), "Debouncer rejected button click, aborting");
                            continue;
                        }
                        ESP_LOGD(_this.tag().c_str(), "Posting event");
                        esp_event_post_to(
                            _this._config.eventLoop,
                            events::SCALE_EVENT,
                            _this._config.eventID,
                            nullptr,
                            0,
                            portMAX_DELAY
                        );
                        ESP_LOGD(_this.tag().c_str(), "Event posted");
                    }
                }
            },
            "Q Handler", 2048, this, 10, nullptr
        );

        gpio_config(&buttonConfig);
        ESP_LOGI(tag().c_str(), "Configuring button on GPIO %d", _config.buttonGPIO);
        gpio_isr_handler_add(_config.buttonGPIO, PushButton::gpio_isr_handler, this);
        ESP_LOGI(tag().c_str(), "ISR Handler installed");
    }

    std::string PushButton::tag() const {
        std::ostringstream oss;
        oss << "Button_" << _config.buttonGPIO;
        return oss.str();
    }

    void IRAM_ATTR PushButton::gpio_isr_handler(void* arg) {
        auto &_this = *(PushButton*)arg;
        xQueueSendFromISR(_this._q, nullptr, 0);
    }
}