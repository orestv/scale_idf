#include "mqtt_report.h"

#include <sstream>

#include "esp_log.h"

namespace scale::mqtt {
    MQTTReport::MQTTReport(const MQTTReportConfig &config):
            _topics(config.topics),
            _mqttClient(config.mqttClient),
            _debouncer(1000) {
        
    }

    void MQTTReport::reportWeight(float grams) {
        std::ostringstream oss;
        oss << grams;

        ESP_LOGI(tag, "Reporting weight %f over MQTT", grams);
        if (!_mqttClient.isConnected()) {
            ESP_LOGW(tag, "MQTT not connected, weight report skipped");
            return;
        }        

        if (!_debouncer.shouldPublish(grams)) {
            ESP_LOGI(tag, "Debouncer blocking report, not sending");
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