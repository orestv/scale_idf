#pragma once

#include <string>
#include <vector>

#include "mqtt_client.h"

#include "scale/events.h"

namespace scale::mqtt {
    struct MQTTConfig {
        std::string brokerUrl;
        esp_event_loop_handle_t eventLoop;
    };

    struct MQTTSubscription {
        std::string topic;
        int qos;
    };

    struct MQTTSubscriptionConfig {
        std::vector<MQTTSubscription> subscriptions;
    };

    struct IncomingMQTTMessage {
        std::string topic;
        std::string message;
        uint8_t qos;
    };

    struct OutgoingMQTTMessage {
        std::string topic;
        std::string message;
        uint8_t qos;

        bool operator==(const OutgoingMQTTMessage &other) const {
            return (
                topic == other.topic &&
                message == other.message &&
                qos == other.qos
            );
        }

        bool operator!=(const OutgoingMQTTMessage &other) const {
            return !(*this == other);
        }
    };

    class MQTTClient {
        public:
            MQTTClient(const MQTTConfig &config, const MQTTSubscriptionConfig &subscriptions);
            void start();

            IncomingMQTTMessage getNextMessage(int timeoutMS);
            void send(OutgoingMQTTMessage message);
            bool isConnected() const;
        private:
            void mqttEventHandler(esp_event_base_t base, int32_t event_id, void *event_data);

            void emitConnectionStateChange() {
                events::EventMQTTConnectionChanged outgoingEvent = {
                    .connected = _isConnected,
                };
                esp_event_post_to(
                    _config.eventLoop,
                    events::SCALE_EVENT,
                    events::EVENT_MQTT_CONNECTION_CHANGED,
                    &outgoingEvent,
                    sizeof(outgoingEvent),
                    portMAX_DELAY);
            }

            MQTTConfig _config;
            MQTTSubscriptionConfig _subscriptions;

            esp_mqtt_client_handle_t _espMqttClient;

            bool _isConnected;
    };
}