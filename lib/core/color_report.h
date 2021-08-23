#pragma once

#include "led.h"

namespace scale::color {
    class ColorReport {
    public:
        ColorReport(scale::led::LED &led) : _led(led) {}

    private:
        scale::led::LED _led;
    };
}