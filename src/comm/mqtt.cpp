#include "scale/mqtt.h"

#include "esp_log.h"

#include "mqtt_client.h"

namespace scale::mqtt {
    const char *TAG = "MQTT";

    static void log_error_if_nonzero(const char *message, int error_code) {
        if (error_code != 0) {
            ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
        }
    }\

    MQTTClient::MQTTClient(const MQTTConfig &config, const MQTTSubscriptionConfig &subscriptions) : _config(config),
                                                                                                    _subscriptions(subscriptions),
                                                                                                    _isConnected(false) {
        esp_mqtt_client_config_t mqtt_cfg = {
            .uri = _config.brokerUrl.c_str(),
        };
        _espMqttClient = esp_mqtt_client_init(&mqtt_cfg);
    }

    void MQTTClient::start() {
        esp_mqtt_client_start(_espMqttClient);
        esp_mqtt_client_register_event(
            _espMqttClient,
            (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID,
            [](void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
            {
                MQTTClient &_this = *(MQTTClient *)handler_args;
                _this.mqttEventHandler(base, event_id, event_data);
            },
            this);
        // esp_mqtt_client_start(_espMqttClient);
        esp_event_handler_register_with(
            _config.eventLoop,
            events::SCALE_EVENT,
            events::EVENT_WIFI_CONNECTION_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(MQTTClient*)arg;
                auto &evt = *(events::EventWifiConnectionChanged*)event_data;
                ESP_LOGI(TAG, "Received wifi update: %s", evt.connected ? "connected" : "disconnected");
                if (evt.connected) {
                    esp_mqtt_client_reconnect(_this._espMqttClient);
                } else {
                    if (_this._isConnected) {
                        // esp_mqtt_client_stop(_this._espMqttClient);
                        _this._isConnected = false;
                        _this.emitConnectionStateChange();
                    }
                }
            }, 
            this
        );
    }

    bool MQTTClient::isConnected() const {
        return _isConnected;
    }

    void MQTTClient::send(OutgoingMQTTMessage message) {
        ESP_LOGD(TAG, "Sending MQTT message to %s: %s", message.topic.c_str(), message.message.c_str());
        esp_mqtt_client_publish(
            _espMqttClient,
            message.topic.c_str(),
            message.message.c_str(),
            0, message.qos, 0
        );
        esp_event_post_to(
            _config.eventLoop,
            events::SCALE_EVENT,
            events::EVENT_WEIGHT_REPORTED,
            nullptr,
            0,
            0
        );
    }

    void MQTTClient::mqttEventHandler(esp_event_base_t base, int32_t event_id, void *event_data) {
        ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
        esp_mqtt_client_handle_t client = event->client;
        int msg_id;
        switch ((esp_mqtt_event_id_t)event_id)
        {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
            _isConnected = true;
            emitConnectionStateChange();
            for (auto const &subscribeTopic : _config.subscribeTopics) {
                esp_mqtt_client_subscribe(client, subscribeTopic.c_str(), 0);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            _isConnected = false;
            ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
            emitConnectionStateChange();
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            // ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA: {
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            events::EventMQTTMessageReceived msg = {
                .topic=std::string(event->topic, event->topic_len),
                .message=std::string(event->data, event->data_len),
            };
            esp_event_post_to(
                _config.eventLoop,
                events::SCALE_EVENT,
                events::EVENT_MQTT_MESSAGE_RECEIVED,
                &msg,
                sizeof(msg),
                portMAX_DELAY
            );
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        }
        case MQTT_EVENT_ERROR:
            ESP_LOGD(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
                ESP_LOGD(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGD(TAG, "Other event id:%d", event->event_id);
            break;
        }
    }
}