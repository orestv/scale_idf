#pragma once

#include "led.h"
#include "scale_events.h"

namespace scale::color {
    struct ColorReportArgs {
        led::LED &led;
        esp_event_loop_handle_t eventLoop;
    };
    class ColorReport {
    public:
        ColorReport(const ColorReportArgs &args) : _led(args.led), _eventLoop(args.eventLoop) {
            
        }

        void onWeightChanged(const float weight);

    private:
        scale::led::LED _led;
        esp_event_loop_handle_t _eventLoop;
    };
}