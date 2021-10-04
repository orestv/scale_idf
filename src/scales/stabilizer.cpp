#include "stabilized_scale.h"

#include <numeric>
#include <math.h>

namespace scale::stabilized {

    const int MAX_UNSTABLE_VALUE_COUNT = 5;

    void Stabilizer::push(float value) {
        _lastValue = value;
        if (!_isNewValueStable(value)) {
            ++_unstableValuesCount;
            if (_unstableValuesCount > MAX_UNSTABLE_VALUE_COUNT) {
                _loseStability();
            }
            return;
        } else {
            _unstableValuesCount = 0;
        }
        if (!isStable()) {
            _values.push_back(value);
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
}