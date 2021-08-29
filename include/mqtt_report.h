#pragma once

#include <string>
#include <mqtt.h>

#include "scale/events.h"

namespace scale::mqtt {
    struct MQTTReportTopics {
        std::string topicWeight;
        std::string topicStable;
    };
    struct MQTTReportConfig {  
        MQTTReportTopics topics;
        MQTTClient &mqttClient;
        esp_event_loop_handle_t eventLoop;
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

    private:
        void reportWeight(float grams);
        void reportStable(bool stable);

        MQTTReportTopics _topics;
        MQTTClient &_mqttClient;
        esp_event_loop_handle_t _eventLoop;

        MQTTDebouncer _debouncer;

        const char *tag = "MQTTReport";
    };
}