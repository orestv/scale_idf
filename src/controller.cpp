#include "controller.h"

#include "esp_log.h"

#include <sstream>

namespace scale::controller {
    const char *TAG = "Controller";

    void ScaleController::start() {
        xTaskCreate(
            [](void *arg) {
                ScaleController &_this = *(ScaleController*)arg;
                while (true) {
                    stabilized::ScaleEvent evt = _this._stabilizedScale.getEvent();
                    _this.processScaleEvent(evt);
                }
            },
            "ScaleEvents", 4096, this, 10, nullptr
        );

        xTaskCreate(
            [](void *arg) {
                ScaleController &_this = *(ScaleController*)arg;
                while (true) {
                    peri::button::ButtonEvent evt = _this._buttonTare.getEvent();
                    _this.onTareButtonPressed();
                }
            },
            "TareButton", 2048, this, 10, nullptr
        );
    }

    void ScaleController::processScaleEvent(const stabilized::ScaleEvent &event) {
        float taredGramsRaw = _tare.tare(event.gramsRaw);
        _colorReport.onWeightChanged(taredGramsRaw);

        if (event.eventType != stabilized::EVENT_STABLE_WEIGHT) {
            _reportDebouncer.clear();
            return;
        }

        if (_taring) {
            ESP_LOGI(TAG, "Taring");
            _tareConfigBuilder.push(event.grams);
            if (!_tareConfigBuilder.isTareReady()) {
                return;
            }
            _tare.update(_tareConfigBuilder.config());
            _tareConfigBuilder.reset();
            _taring = false;
            _reportDebouncer.clear();
        }

        float taredGrams = _tare.tare(event.grams);
        ESP_LOGI(TAG, "                  %.1fg", taredGrams);
        if (_mqttClient.isConnected()) {
            std::ostringstream oss;
            oss << taredGrams;
            ESP_LOGI(TAG, "Will send %s to %s", oss.str().c_str(), "/feeder/weight");
            mqtt::OutgoingMQTTMessage msg = {
                .topic="/feeder/weight",
                .message=oss.str(),
                .qos=1,
            };
            if (_reportDebouncer.shouldPublish(msg)) {
                _mqttClient.send(msg);
                _reportDebouncer.published(msg);
            }
        }
    }

    void ScaleController::onTareButtonPressed() {
        ESP_LOGI(TAG, "Taring.");
        _taring = true;
        _tareConfigBuilder.reset();
    }

    MQTTReportDebouncer::MQTTReportDebouncer(uint periodMS): _periodMS(periodMS), _published(false) {

    }

    bool MQTTReportDebouncer::shouldPublish(const mqtt::OutgoingMQTTMessage &msg) const {
        if (!_published) return true;
        if (msg != _storedMessage) return true;
        if (now() - _lastPublishTime > _periodMS) return true;
        return false;
    }

    void MQTTReportDebouncer::published(const mqtt::OutgoingMQTTMessage &msg) {
        _lastPublishTime = now();
        _published = true;
        _storedMessage = msg;
    }

    void MQTTReportDebouncer::clear() {
        _published = false;
        _lastPublishTime = 0;
    }

    uint MQTTReportDebouncer::now() const {
        return esp_timer_get_time() / 1000;
    }
}