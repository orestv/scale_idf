#include "scale/presence.h"

namespace scale::presence {
PresenceDetector::PresenceDetector(const PresenceConfig &config) : _config(config) {
    gpio_config_t buttonConfig = {
        .pin_bit_mask = 1ULL << _config.detectorGPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };
    gpio_config(&buttonConfig);
    gpio_isr_handler_add(
        _config.detectorGPIO, [](void *arg) {
            auto &_this = *(PresenceDetector*)arg;
            esp_event_isr_post_to(
                _this._config.eventLoop,
                events::SCALE_EVENT,
                events::EVENT_MOVEMENT_DETECTED,
                nullptr, 0, nullptr
            );
        }, this
    );
}
}  // namespace scale::presence