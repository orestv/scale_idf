#include "scale_button.h"

#include "esp_log.h"

namespace scale::peri::button {
    const char *TAG = "Debouncer";
    bool Debouncer::trigger() {
        ESP_LOGI(TAG, "Checking time");
        uint32_t now = esp_timer_get_time() / 1000;
        ESP_LOGI(TAG, "Now is %i", now);
        uint32_t timeSinceLastTrigger = now - _lastTriggerTime;
        ESP_LOGI(TAG, "Time since last trigger %i", timeSinceLastTrigger);
        if (_lastTriggerTime == 0 || timeSinceLastTrigger > _debounceMillis) {
            ESP_LOGI(TAG, "Returning true");
            _lastTriggerTime = now;
            return true;
        }
        ESP_LOGI(TAG, "Returning false");
        return false;
    }
}