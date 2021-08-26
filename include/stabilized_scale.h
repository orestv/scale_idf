#pragma once

#include "adapted_scale.h"
#include "stabilizer.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace scale::stabilized {
    enum EventType {
        EVENT_STABLE_WEIGHT,
        EVENT_UNSTABLE,
    };

    struct ScaleEvent {
        EventType eventType;
        float grams;
        float gramsRaw;
    };

    class StabilizedScale {
    public:
        StabilizedScale(adapted::AdaptedScale &adaptedScale, Stabilizer &stabilizer): 
                _adaptedScale(adaptedScale),
                _stabilizer(stabilizer) {
            _eventQueue = xQueueCreate(10, sizeof(ScaleEvent));
        }
        void start();
        ScaleEvent getEvent();
    private:
        void task();
        void processEvent(const adapted::ScaleEvent &incomingEvent);
        xQueueHandle _eventQueue;
        adapted::AdaptedScale _adaptedScale;
        Stabilizer _stabilizer;
    };
}