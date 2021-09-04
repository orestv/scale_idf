#pragma once

#include "esp_ota_ops.h"
#include "esp_https_ota.h"

#include "scale/events.h"

namespace scale::ota {
struct OTAConfig {
    esp_event_loop_handle_t eventLoop;
} ;
class OTA {
   public:
    OTA(const OTAConfig &config);
    void start();

   private:
    OTAConfig _config;
    xTaskHandle _otaTask;
};
}  // namespace scale::ota