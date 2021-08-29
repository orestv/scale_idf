#pragma once

#include "stabilizer.h"

#include "scale_events.h"

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
        StabilizedScale(Stabilizer &stabilizer, esp_event_loop_handle_t eventLoop): 
                _eventLoop(eventLoop),
                _stabilizer(stabilizer) {
            _eventQueue = xQueueCreate(10, sizeof(ScaleEvent));
        }
        void start();
        ScaleEvent getEvent();
    private:
        void task();
        void processEvent(const events::EventRawWeightChanged &incomingEvent);
        xQueueHandle _eventQueue;
        esp_event_loop_handle_t _eventLoop;
        Stabilizer _stabilizer;
    };
}