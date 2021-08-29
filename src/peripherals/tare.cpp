#include "scale/tare.h"

namespace scale::tare {
    void TareConfigBuilder::push(float grams) {
        _tareBuffer.push_back(grams);
    }

    bool TareConfigBuilder::isTareReady() const {
        return _tareBuffer.size() >= MAX_BUFFER_SIZE;
    }

    void TareConfigBuilder::reset() {
        _tareBuffer.clear();
    }

    TareConfig TareConfigBuilder::config() const {
        float sum = 0;
        for (const float &t : _tareBuffer) {
            sum += t;
        }
        float tare = sum / _tareBuffer.size();
        TareConfig result;
        result.tareGrams = tare;
        return result;
    }
}