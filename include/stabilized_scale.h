#pragma once

#include "adapted_scale.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace scale::stabilized {
    enum EventType {
        EVENT_WEIGHT,
        EVENT_UNSTABLE,
    };

    struct ScaleEvent {
        EventType eventType;
        float grams = 0;
    };

    class StabilizedScale {
    public:
        StabilizedScale(adapted::AdaptedScale &adaptedScale): _adaptedScale(adaptedScale) {}

        ScaleEvent getEvent();
    private:
        void start();
        void task();
        xQueueHandle _eventQueue;
        adapted::AdaptedScale _adaptedScale;
    };
}