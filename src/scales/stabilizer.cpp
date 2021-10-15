#include "stabilized_scale.h"

#include <numeric>
#include <math.h>
#include "esp_log.h"

namespace scale::stabilized {

    const int MAX_UNSTABLE_VALUE_COUNT = 5;
    const float MAX_DRIFT = 0.8;
    const char *STAB_TAG = "Stabilizer";

    void Stabilizer::push(float value) {
        ESP_LOGI(STAB_TAG, "Pushed weight %f", value);
        _lastValue = value;
        if (!_isNewValueStable(value)) {
            ESP_LOGI(STAB_TAG, "New weight not stable");
            ++_unstableValuesCount;
            if (_unstableValuesCount > MAX_UNSTABLE_VALUE_COUNT) {
                ESP_LOGI(STAB_TAG, "New weight over stability threshold, stability lost");
                _loseStability();
            }
            return;
        } else {
            _unstableValuesCount = 0;
        }
        if (!isStable()) {
            _values.push_back(value);
            return;
        }
        _driftValues.push_back(value);
        if (_driftValues.size() > _driftElementCount()) {
            _driftValues.pop_front();
        }
        if (_driftValues.size() < _driftElementCount()) {
            return;
        }

        float headAverage = getValue();
        float tailAverage = _tailAverage();
        float drift = tailAverage - headAverage;
        ESP_LOGI(STAB_TAG, "Head %.2f, tail %.2f, drift %.2f", headAverage, tailAverage, drift);
        if (abs(drift) > MAX_DRIFT && abs(drift) < _config.margin) {
            ESP_LOGI(STAB_TAG, "Drift of %f exceeds %f", drift, MAX_DRIFT);
            events::EventDriftDetected eventData = {
                .drift=drift,
            };
            esp_event_post_to(
                _config.eventLoop,
                events::SCALE_EVENT,
                events::EVENT_DRIFT_DETECTED,
                &eventData,
                sizeof(eventData),
                pdMS_TO_TICKS(300)
            );
            _loseStability();
        }
    }
    bool Stabilizer::isStable() const {
        return _values.size() >= _config.dataPoints;
    }
    float Stabilizer::getValue() const {
        return _currentAverage();
    }

    void Stabilizer::_loseStability() {
        _values.clear();
        _driftValues.clear();
        _unstableValuesCount = 0;
    }

    float Stabilizer::_currentAverage() const {
        if (_values.size() == 0) return _lastValue;

        float sum = std::accumulate(_values.begin(), _values.end(), 0.0);
        return sum / _values.size();
    }

    bool Stabilizer::_isNewValueStable(float newValue) const {
        if (_values.size() == 0) return true;

        float currentAvg = _currentAverage();
        return abs(currentAvg - newValue) < _config.margin;
    }

    float Stabilizer::_headAverage() const {
        auto iter = _values.begin();
        for (int i = 0; i < _driftElementCount(); i++) {
            iter++;
        }
        float sum = std::accumulate(_values.begin(), iter, 0.0);
        return sum/(float)_driftElementCount();
    }
    float Stabilizer::_tailAverage() const {
        return std::accumulate(_driftValues.begin(), _driftValues.end(), 0.0)/(float)_driftValues.size();
    }
}