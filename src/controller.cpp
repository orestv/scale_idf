#include "controller.h"

#include "esp_log.h"

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
            "ScaleEvents", 2048, this, 10, nullptr
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
        ESP_LOGI(TAG, "                  %fg", taredGrams);
    }

    void ScaleController::onTareButtonPressed() {
        ESP_LOGI(TAG, "Taring.");
        _taring = true;
        _tareConfigBuilder.reset();
    }
}