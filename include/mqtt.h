#pragma once

#include <string>
#include <vector>

#include "mqtt_client.h"

namespace scale::mqtt {
    struct MQTTConfig {
        std::string brokerUrl;
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

            IncomingMQTTMessage getNextMessage(int timeoutMS);
            void send(OutgoingMQTTMessage message);
            bool isConnected() const;
        private:
            void mqttEventHandler(esp_event_base_t base, int32_t event_id, void *event_data);

            MQTTConfig _config;
            MQTTSubscriptionConfig _subscriptions;

            esp_mqtt_client_handle_t _espMqttClient;

            bool _isConnected;
    };
}