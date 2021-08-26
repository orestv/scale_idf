#include "weight_sensor.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "esp_log.h"

namespace scale::raw {

    void Scale::start() {
        ESP_LOGI(tag().c_str(), "Starting the scale");

        while (1) {
            esp_err_t r = hx711_init(&_dev);
            if (r == ESP_OK)
                break;
            ESP_LOGI(tag().c_str(), "Could not initialize HX711: %d (%s)\n", r, esp_err_to_name(r));
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        xTaskCreate(Scale::process, "Scale Processor", 2048, this, 10, nullptr);
    }

    void Scale::process(void *arg) {
        Scale *_this = (Scale*)arg;
        ESP_LOGI(_this->tag().c_str(), "Starting task process...");

        while (true) {
            esp_err_t r = hx711_wait(&_this->_dev, 10);
            if (r != ESP_OK) {
                ESP_LOGE(_this->tag().c_str(), "Device not found: %d (%s)", r, esp_err_to_name(r));
                continue;
            }
            
            int32_t data;
            r = hx711_read_data(&_this->_dev, &data);
            if (r != ESP_OK) {
                ESP_LOGE(_this->tag().c_str(), "Could not read data: %d (%s)", r, esp_err_to_name(r));
                continue;
            }
            ScaleEvent event = {
                .rawData = data,
            };
            xQueueSend(_this->queue(), &event, portMAX_DELAY);
            // ESP_LOGI(_this->tag().c_str(), "Raw data: %d", data);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }

    std::string Scale::tag() const {
        return "Scale";
    }
}