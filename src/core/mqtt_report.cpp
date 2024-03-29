#include "mqtt_report.h"

#include <sstream>

#include "esp_log.h"

namespace scale::mqtt {
    MQTTReport::MQTTReport(const MQTTReportConfig &config):
            _topics(config.topics),
            _mqttClient(config.mqttClient),
            _maintenance(config.maintenance),
            _eventLoop(config.eventLoop),
            _debouncer(1000) {
        
        esp_event_handler_register_with(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_STABILIZED_TARED_WEIGHT_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(MQTTReport*)arg;
                auto &event = *(events::EventStabilizedTaredWeightChanged*)event_data;

                _this.reportWeight(event.grams);
            },
            this
        );
        esp_event_handler_register_with(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_LOG_MESSAGE,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(MQTTReport *)arg;
                auto &event = *(events::EventLogMessage *)event_data;

                _this.reportLog(event.logMessage);
            },
            this
        );
    }

    void MQTTReport::reportWeight(float grams) {
        std::ostringstream oss;
        oss << grams;

        if (_maintenance.isMaintenanceModeOn()) {
            ESP_LOGD(tag, "Maintenance mode on: not reporting weight");
            return;
        }

        ESP_LOGD(tag, "Reporting weight %f over MQTT", grams);
        if (!_mqttClient.isConnected()) {
            ESP_LOGD(tag, "MQTT not connected, weight report skipped");
            return;
        }        

        if (!_debouncer.shouldPublish(grams)) {
            ESP_LOGD(tag, "Debouncer blocking report, not sending");
            return;
        }

        ESP_LOGI(tag, "Sending MQTT report of weight %f", grams);
        OutgoingMQTTMessage msg{
            .topic=_topics.topicWeight,
            .message=oss.str(),
            .qos=1,
        };
        _mqttClient.send(msg);
        _debouncer.published(grams);
    }

    void MQTTReport::reportStable(bool stable) {
        if (!stable) {
            _debouncer.clear();
        }
    }

    void MQTTReport::reportLog(const std::string &message) {
        if (!_mqttClient.isConnected()) {
            return;
        }
        OutgoingMQTTMessage msg{
            .topic="/feeder/log",
            .message=message,
            .qos=0,
        };
        _mqttClient.send(msg);
    }

    MQTTDebouncer::MQTTDebouncer(uint periodMS) : _periodMS(periodMS), _published(false)
    {
    }

    bool MQTTDebouncer::shouldPublish(float grams) const
    {
        if (!_published)
            return true;
        if (grams != _storedGrams)
            return true;
        if (now() - _lastPublishTime > _periodMS)
            return true;
        return false;
    }

    void MQTTDebouncer::published(float grams)
    {
        _lastPublishTime = now();
        _published = true;
        _storedGrams = grams;
    }

    void MQTTDebouncer::clear()
    {
        _published = false;
        _lastPublishTime = 0;
    }

    uint MQTTDebouncer::now() const
    {
        return esp_timer_get_time() / 1000;
    }
}