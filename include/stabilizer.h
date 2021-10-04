#pragma once

#include <deque>

namespace scale::stabilized {
    struct StabilizerConfig {
        int dataPoints;
        float margin;
    };

    class Stabilizer {
    public:
        Stabilizer(const StabilizerConfig &config): _config(config) {}

        float getValue() const;
        bool isStable() const;
        void push(float value);
    private:
        float _currentAverage() const;

        bool _isNewValueStable(float newValue) const;
        void _loseStability();

        float _lastValue = 0;
        int _currentIndex = 0;

        int _unstableValuesCount = 0;

        std::deque<float> _values;
        StabilizerConfig _config;
    };

}