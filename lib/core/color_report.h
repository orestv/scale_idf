#pragma once

#include "led.h"

namespace scale::color {
    class ColorReport {
    public:
        ColorReport(scale::led::LED &led) : _led(led) {}

        void onWeightChanged(const float weight);

    private:
        scale::led::LED _led;
    };
}