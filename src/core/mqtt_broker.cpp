#include <string>

#include "scale/mqttBroker.h"

#include "esp_log.h"

namespace scale::mqtt {
const char *TAG_BROKER = "Broker";
MQTTBroker::MQTTBroker(const MQTTBrokerConfig &config) : _config(config) {
    esp_event_handler_register_with(
        _config.eventLoop,
        events::SCALE_EVENT,
        events::EVENT_MQTT_MESSAGE_RECEIVED,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(MQTTBroker *)arg;
            auto &evt = *(events::EventMQTTMessageReceived *)event_data;
            ESP_LOGI(
                TAG_BROKER, 
                "Received MQTT event on topic %s, payload %s", 
                evt.topic.c_str(), 
                evt.message.c_str()
            );
            if (evt.topic == _this._config.topicUpdate) {
                esp_event_post_to(
                    _this._config.eventLoop,
                    events::SCALE_EVENT,
                    events::EVENT_OTA_REQUESTED,
                    nullptr, 0, portMAX_DELAY);
            }
        },
        this);
}

}  // namespace scale::mqtt