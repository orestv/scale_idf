#pragma once

#include <string>

#include <freertos/FreeRTOS.h>
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_netif.h"

namespace scale::wifi {
    struct AP {
        std::string ssid;
        std::string wpa2Password;
    };

    struct WifiEvents {
        bool isConnected = false;
    };

    class WifiClient {
    public:
        WifiClient();
        void start(AP accessPoint, size_t accessPointsLen);

        EventGroupHandle_t eventGroup() {
            return this->_wifiEventGroup;
        }
    private:
        static void static_got_ip_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void got_ip_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        static void static_wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

        EventGroupHandle_t _wifiEventGroup;
    };
}