#include "scale/ota.h"

#include <string>

#include "esp_log.h"
#include "scale/events.h"
#include "stdio.h"

#define HTTP_BUF_SIZE (8192 * 4)

namespace scale::ota {
const char *TAG = "OTA";

using namespace std;

esp_err_t OTA::httpEventHandler(esp_http_client_event_t *evt) {
    static int content_length = 0;
    static int collected_data = 0;

    string hdr;
    string exp("Content-Length");
    static int lastPct = -1;
    int pct = 0;

    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            hdr = evt->header_key;
            if (hdr == exp) {
                content_length = atoi(evt->header_value);
            }
            break;
        case HTTP_EVENT_ON_DATA:
            collected_data += evt->data_len;
            if (content_length > 0) {
                pct = 100 * (float)collected_data / content_length;
                if (pct != lastPct) {
                    ESP_LOGI(TAG, "Downloading OTA, %d%% done", pct);
                    lastPct = pct;
                    emitUpdatePercentageChanged(pct);
                }
            }
            // ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            collected_data = 0;
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            collected_data = 0;
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

OTA::OTA(const OTAConfig &config) : _config(config) {
    esp_event_handler_register_with(
        _config.eventLoop,
        events::SCALE_EVENT,
        ESP_EVENT_ANY_ID,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(OTA *)arg;

            switch (event_id) {
                case events::EVENT_MQTT_MESSAGE_RECEIVED: {
                    xTaskNotifyGive(_this._otaTask);
                }
                case events::EVENT_MAINTENANCE_MODE_CHANGED: {
                    ESP_LOGI(TAG, "Caught maintenance change");
                    auto &event = *(events::EventMaintenanceModeChanged *)event_data;
                    if (event.isMaintenanceModeOn) {
                        xTaskNotifyGive(_this._otaTask);
                    }
                    break;
                }
                case events::EVENT_WIFI_CONNECTION_CHANGED: {
                    ESP_LOGI(TAG, "Caught wifi state change");
                    auto &event = *(events::EventWifiConnectionChanged *)event_data;
                    break;
                }
            }
        },
        this);
}

void OTA::emitUpdateStarted() {
    events::EventUpdateStateChange eventData = {
        .isUpdating = true,
        .updatePercentage = 0,
    };
    emitEvent(eventData);
}

void OTA::emitUpdatePercentageChanged(int pct) {
    events::EventUpdateStateChange eventData = {
        .isUpdating = true,
        .updatePercentage = pct,
    };
    emitEvent(eventData);
}

void OTA::emitUpdateComplete(bool isSuccess) {
    events::EventUpdateStateChange eventData = {
        .isUpdating = false,
        .updatePercentage = 0,
    };
    emitEvent(eventData);
}

void OTA::emitEvent(events::EventUpdateStateChange eventData) {
    esp_event_post_to(
        _config.eventLoop,
        events::SCALE_EVENT,
        events::EVENT_UPDATE_STATE_CHANGE,
        &eventData,
        sizeof(eventData),
        portMAX_DELAY);
}

void OTA::start() {
    auto appDescription = esp_ota_get_app_description();
    ESP_LOGI(TAG, "Current FW version: %s", appDescription->version);
    xTaskCreate(
        [](void *arg) {
            auto &_this = *(OTA*)arg;
            auto httpEventHandler = [](esp_http_client_event_t *evt) {
                auto &_this = *(OTA *)evt->user_data;
                return _this.httpEventHandler(evt);
            };
            while (true) {
                ulTaskNotifyTake(1, portMAX_DELAY);
                ESP_LOGI(TAG, "Trying to run OTA");
                
                _this.emitUpdateStarted();

                esp_http_client_config_t otaConfig = {
                    .url = "http://openhab/firmware/firmware.bin",
                    .cert_pem = "",
                    .event_handler = httpEventHandler,
                    .buffer_size = HTTP_BUF_SIZE,
                    .user_data = arg,
                    .skip_cert_common_name_check = true,
                    .keep_alive_enable = true,
                };
                esp_err_t ret = esp_https_ota(&otaConfig);
                if (ret == ESP_OK) {
                    esp_restart();
                } else {
                    ESP_LOGE(TAG, "Firmware upgrade failed");
                    _this.emitUpdateComplete(false);
                }
            }
        },
        "OTA", 4096 + HTTP_BUF_SIZE, this, 5, &_otaTask);
}
}  // namespace scale::ota