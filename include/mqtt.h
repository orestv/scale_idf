#pragma once

#include <string>

namespace scale::mqtt {
    struct MQTTParams {
        std::string brokerUrl;
    };

    struct IncomingMQTTMessage {
        std::string topic;
        std::string message;
        uint8_t qos;
    };

    class MQTTClient {
        public:
            MQTTClient();

            IncomingMQTTMessage getNextMessage(int timeoutMS);
        private:

    };
}