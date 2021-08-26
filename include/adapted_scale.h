#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <vector>

#include "weight_sensor.h"
#include "weight_converter.h"

namespace scale::adapted {
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

        xQueueHandle queue() const {
            return _eventQueue;
        }   

    private:
        void start();

        ScaleEvent convertEvent(const raw::ScaleEvent &incomingEvent) const;
        void processEvent(const raw::ScaleEvent &incomingEvent);

        static void processTask(void *arg);

        raw::Scale &_scale;
        raw::GramConverter &_converter;

        xQueueHandle _eventQueue;
    };
}