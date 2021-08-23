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
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .intr_type = GPIO_INTR_POSEDGE,
        };

        gpio_config(&buttonConfig);
        ESP_LOGI(tag().c_str(), "Configuring button on GPIO %d", _config.buttonGPIO);
        gpio_isr_handler_add(_config.buttonGPIO, PushButton::gpio_isr_handler, this);
        ESP_LOGI(tag().c_str(), "ISR Handler installed");

        xTaskCreate(PushButton::gpioTask, tag().c_str(), 2048, this, 10, nullptr);
    }

    std::string PushButton::tag() const {
        std::ostringstream oss;
        oss << "Button_" << _config.buttonGPIO;
        return oss.str();
    }

    void PushButton::gpioTask(void *arg) {
        PushButton *_this = (PushButton*)arg;
        uint32_t io_num;
        while (true) {
            if (xQueueReceive(_this->_buttonEventQueue, &io_num, portMAX_DELAY)) {
                ESP_LOGI(_this->tag().c_str(), "Button clicked: %d", io_num);
            }
        }
    }

    void IRAM_ATTR PushButton::gpio_isr_handler(void* arg) {
        PushButton *_this = (PushButton*)arg;
        if (_this->_debouncer.trigger()) {
            uint32_t gpio_num = _this->_config.buttonGPIO;
            xQueueSendFromISR(_this->_buttonEventQueue, &gpio_num, NULL);
        }
    }
}