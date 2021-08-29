#pragma once

#include <string>
#include "scale/mqtt.h"
#include "scale/events.h"

#include "maintenance.h"

namespace scale::mqtt {
    struct MQTTReportTopics {
        std::string topicWeight;
        std::string topicStable;
    };
    struct MQTTReportConfig {  
        MQTTReportTopics topics;
        MQTTClient &mqttClient;
        maintenance::Maintenance &maintenance;
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
        maintenance::Maintenance &_maintenance;
        esp_event_loop_handle_t _eventLoop;

        MQTTDebouncer _debouncer;

        const char *tag = "MQTTReport";
    };
}