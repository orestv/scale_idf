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
    esp_err_t httpEventHandler(esp_http_client_event_t *evt);

    void emitUpdateStarted();
    void emitUpdatePercentageChanged(int pct);
    void emitUpdateComplete(bool isSuccess);

    void emitEvent(events::EventUpdateStateChange event);

    OTAConfig _config;
    xTaskHandle _otaTask;
};
}  // namespace scale::ota