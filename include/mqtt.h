#pragma once

#include <string>
#include <vector>

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

    class MQTTClient {
        public:
            MQTTClient(const MQTTConfig &config, const MQTTSubscriptionConfig &subscriptions);

            IncomingMQTTMessage getNextMessage(int timeoutMS);
        private:
            MQTTConfig _config;
            MQTTSubscriptionConfig _subscriptions;
    };
}