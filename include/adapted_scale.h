#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <vector>

#include "raw_scale.h"
#include "weight_converter.h"

#include "scale/events.h"

namespace scale::adapted {
    struct ScaleEvent {
        float grams;
    };

    class AdaptedScale {
    public:
        AdaptedScale(raw::Scale &scale, raw::GramConverter &converter, esp_event_loop_handle_t eventLoop):
            _scale(scale), 
            _converter(converter),
            _eventLoop(eventLoop) {

            start();
        }

    private:
        void start();

        events::EventRawWeightChanged convertEvent(const raw::ScaleEvent &incomingEvent) const;
        void processEvent(const raw::ScaleEvent &incomingEvent);

        void task();

        raw::Scale &_scale;
        raw::GramConverter &_converter;

        esp_event_loop_handle_t _eventLoop;
    };
}