#include "scale_button.h"

namespace scale::peri::button {
    bool Debouncer::trigger() {
        uint32_t now = esp_timer_get_time() / 1000;
        uint32_t timeSinceLastTrigger = now - _lastTriggerTime;
        if (_lastTriggerTime == 0 || timeSinceLastTrigger > _debounceMillis) {
            _lastTriggerTime = now;
            return true;
        }
        return false;
    }
}