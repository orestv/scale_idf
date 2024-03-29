#pragma once

#include <string>

#include <freertos/FreeRTOS.h>
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_netif.h"

#include "scale/events.h"

namespace scale::wifi {
    struct AP {
        std::string ssid;
        std::string wpa2Password;
    };

    struct WifiConfig {
        std::string hostname;
        esp_event_loop_handle_t eventLoop;
    };

    struct WifiEvents {
        bool isConnected = false;
    };

    class WifiClient {
    public:
        WifiClient(const WifiConfig &wifiConfig);
        void start(AP accessPoint, size_t accessPointsLen);
    private:
        static void static_got_ip_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void got_ip_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        static void static_wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

        WifiConfig _wifiConfig;
        EventGroupHandle_t _wifiEventGroup;
    };
}