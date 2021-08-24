#include "wifi.h"

#include <stdio.h>
#include <string.h>
#include <functional>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_netif.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

namespace scale::wifi {

    const char *TAG = "WIFI";



    void WifiClient::static_got_ip_handler(
        void* arg, esp_event_base_t event_base,
        int32_t event_id, void* event_data) {

        WifiClient *wifiClient = (WifiClient*)arg;
        wifiClient->got_ip_handler(arg, event_base, event_id, event_data);
    }

    void WifiClient::got_ip_handler(
        void* arg, esp_event_base_t event_base,
        int32_t event_id, void* event_data) {
        
        ESP_LOGI(TAG, "Wifi Connected event");

        if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(this->eventGroup(), WIFI_CONNECTED_BIT);
        }
    }

    void WifiClient::static_wifi_event_handler(
        void* arg, esp_event_base_t event_base,
        int32_t event_id, void* event_data) {

        WifiClient *wifiClient = (WifiClient*)arg;
        wifiClient->wifi_event_handler(arg, event_base, event_id, event_data);
    }

    void WifiClient::wifi_event_handler(
        void* arg, esp_event_base_t event_base,
        int32_t event_id, void* event_data) {
        
        ESP_LOGI(TAG, "Got Wifi event");

        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
            return;
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            esp_wifi_connect();
        }
    }

    WifiClient::WifiClient(const WifiConfig &wifiConfig) {
        this->_wifiConfig = wifiConfig;

        this->_wifiEventGroup = xEventGroupCreate();

        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &WifiClient::static_wifi_event_handler,
                                                            this,
                                                            nullptr));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &WifiClient::static_got_ip_handler,
                                                            this,
                                                            nullptr));


    }

    template<int size>
    void cstr_to_uint8(const char *src, uint8_t (&dst)[size]) {
        auto srcLen = strlen(src);
        for (int i = 0; i < srcLen; i++) {
            dst[i] = src[i];
        }
    }

    void WifiClient::start(AP accessPoint, size_t accessPointsLen) {
        esp_netif_create_default_wifi_sta();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        wifi_config_t wifi_config = {
            .sta = {
                .pmf_cfg = {
                    .capable = true,
                    .required = false
                },
            },
        };

        cstr_to_uint8<32>(accessPoint.ssid.c_str(), wifi_config.sta.ssid);
        cstr_to_uint8<64>(accessPoint.wpa2Password.c_str(), wifi_config.sta.password);

        ESP_LOGI(TAG, "SSID: %s", wifi_config.sta.ssid);
        ESP_LOGI(TAG, "PWD: %s", wifi_config.sta.ssid);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        ESP_ERROR_CHECK(esp_wifi_start() );

        esp_err_t err_set_hostname = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, _wifiConfig.hostname.c_str());
        if (err_set_hostname != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get hostname: %d", err_set_hostname);
        }

        EventBits_t bits = xEventGroupWaitBits(
            this->_wifiEventGroup,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "CONNECTED TO WIFI");
        }
    }
}