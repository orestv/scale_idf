#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "weight_sensor.h"
#include "weight_converter.h"

namespace scale::weight {
    struct ScaleEvent {
        float grams;
    };

    class AdaptedScale {
    public:
        AdaptedScale(raw::Scale &scale, raw::GramConverter &converter):
            _scale(scale), 
            _converter(converter) {
            
            _eventQueue = xQueueCreate(20, sizeof(ScaleEvent));

            start();
        }

        void tare();     

        xQueueHandle queue() const {
            return _eventQueue;
        }   

    private:
        void start();

        ScaleEvent convertEvent(const raw::ScaleEvent &incomingEvent) const;

        static void processTask(void *arg);

        raw::Scale &_scale;
        raw::GramConverter &_converter;

        xQueueHandle _eventQueue;
    };
}