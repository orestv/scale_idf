#pragma once

#include <deque>
#include "esp_event.h"

namespace scale::stabilized {
    struct StabilizerConfig {
        int dataPoints;
        float margin;
        esp_event_loop_handle_t eventLoop;
    };

    class Stabilizer {
    public:
        Stabilizer(const StabilizerConfig &config): _config(config) {}

        float getValue() const;
        bool isStable() const;
        void push(float value);
    private:
        float _currentAverage() const;
        float _headAverage() const;
        float _tailAverage() const;

        bool _isNewValueStable(float newValue) const;
        void _loseStability();

        int _driftElementCount() const {
            return _config.dataPoints / 3;
        }

        float _lastValue = 0;
        int _currentIndex = 0;

        int _unstableValuesCount = 0;

        std::deque<float> _values;
        std::deque<float> _driftValues;
        StabilizerConfig _config;
    };

}