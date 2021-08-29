#pragma once

#include "led.h"
#include "scale/events.h"

namespace scale::color {
    struct ColorReportArgs {
        led::LED &led;
        esp_event_loop_handle_t eventLoop;
    };
    class ColorReport {
    public:
        ColorReport(const ColorReportArgs &args);

    private:
     void onWeightChanged(const float weight);

     scale::led::LED _led;
     esp_event_loop_handle_t _eventLoop;
    };
}