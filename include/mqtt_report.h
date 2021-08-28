#pragma once

#include <string>
#include <mqtt.h>

namespace scale::mqtt {
    struct MQTTReportTopics {
        std::string topicWeight;
        std::string topicStable;
    };
    struct MQTTReportConfig {  
        MQTTReportTopics topics;
        MQTTClient &mqttClient;
    };

    class MQTTDebouncer {
    public:
        MQTTDebouncer(uint periodMS);
        bool shouldPublish(float grams) const;
        void published(float grams);
        void clear();

    private:
        uint now() const;

        uint _periodMS;

        float _storedGrams;
        bool _published;
        int _lastPublishTime;
    };

    class MQTTReport {
    public:
        MQTTReport(const MQTTReportConfig &config);

        void reportWeight(float grams);
        void reportStable(bool stable);
    private:
        MQTTReportTopics _topics;
        MQTTClient &_mqttClient;

        MQTTDebouncer _debouncer;

        const char *tag = "MQTTReport";
    };
}