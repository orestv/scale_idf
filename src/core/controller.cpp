#include "scale/controller.h"

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
        esp_event_handler_register_with(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_RAW_WEIGHT_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
                ScaleController &_this = *(ScaleController *)arg;
                events::EventRawWeightChanged evt = *(events::EventRawWeightChanged*)event_data;
                _this.onRawWeightChanged(evt);
            },
            this);

        xTaskCreate(
            [](void *arg)
            {
                ScaleController &_this = *(ScaleController *)arg;
                while (true)
                {
                    peri::button::ButtonEvent evt = _this._buttonTare.getEvent();
                    _this.onTareButtonPressed();
                }
            },
            "TareButton", 2048, this, 10, nullptr);

        xTaskCreate(
            [](void *arg) {
                ScaleController &_this = *(ScaleController*)arg;
                while (true) {
                    peri::button::ButtonEvent evt = _this._buttonMaintenance.getEvent();
                    _this.onMaintenanceButtonPressed();
                }
            },
            "MaintenanceButton", 2048, this, 10, nullptr
        );
    }

    void ScaleController::processScaleEvent(const stabilized::ScaleEvent &event) {
        return;
        float taredGramsRaw = _tare.tare(event.gramsRaw);
        // _colorReport.onWeightChanged(taredGramsRaw);

        if (event.eventType != stabilized::EVENT_STABLE_WEIGHT) {
            _mqttReport.reportStable(false);
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
        }

        float taredGrams = _tare.tare(event.grams);
        ESP_LOGI(TAG, "                  %.1fg", taredGrams);

        _lcd.setWeight(taredGrams);
        if (!_maintenance.isMaintenanceModeOn()) {
            _mqttReport.reportWeight(taredGrams);        
        } else {
            ESP_LOGI(TAG, "Maintenance mode on: not reporting weight");
        }
    }

    void ScaleController::onRawWeightChanged(const events::EventRawWeightChanged &event) {
        ESP_LOGD(TAG, "Raw weight changed");
        if (_taring) {
            ESP_LOGD(TAG, "Taring");
            _tareConfigBuilder.push(event.grams);
            if (_tareConfigBuilder.isTareReady()) {
                _tare.update(_tareConfigBuilder.config());
                _tareConfigBuilder.reset();
                _taring = false;
                events::EventTareComplete evtTareComplete = {};
                esp_event_post_to(
                    _eventLoop,
                    events::SCALE_EVENT,
                    events::EVENT_TARE_COMPLETE,
                    &evtTareComplete,
                    sizeof(evtTareComplete),
                    portMAX_DELAY
                );
            } else {
                return;
            }
        }
        
        ESP_LOGD(TAG, "Sending raw tared weight changed event");
        float taredRawWeight = _tare.tare(event.grams);
        events::EventRawTaredWeightChanged eventWeightChanged = {
            .grams = taredRawWeight,
        };
        esp_event_post_to(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_RAW_TARED_WEIGHT_CHANGED,
            &eventWeightChanged,
            sizeof(eventWeightChanged),
            portMAX_DELAY
        );
    }

    void ScaleController::onTareButtonPressed() {
        ESP_LOGI(TAG, "Taring.");
        _taring = true;
        _tareConfigBuilder.reset();
    }

    void ScaleController::onMaintenanceButtonPressed() {
        ESP_LOGI(TAG, "Toggling maintenance mode");
        _maintenance.toggleMaintenanceMode();
        ESP_LOGI(TAG, "Maintenance mode is now %s", _maintenance.isMaintenanceModeOn() ? "on" : "off");
    }
}