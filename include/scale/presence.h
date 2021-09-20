#pragma once

#include <driver/gpio.h>

#include "scale/events.h"

namespace scale::presence {
struct PresenceConfig {
    gpio_num_t detectorGPIO;
    esp_event_loop_handle_t eventLoop;
};

class PresenceDetector {
   public:
    PresenceDetector(const PresenceConfig &config);

   private:
    PresenceConfig _config;
};
}  // namespace scale::presence