#include "stabilized_scale.h"

namespace scale::stabilized {

    void StabilizedScale::start() {
        xTaskCreate(
            [](void *arg) {
                ((StabilizedScale*)arg)->task();
            },
            "StabilizedScale", 2048, this, 10, nullptr);
    }

    void StabilizedScale::task() {
        while (true) {
            scale::adapted::ScaleEvent incomingEvent = _adaptedScale.getEvent();
        }        
    }

    ScaleEvent StabilizedScale::getEvent() {
        ScaleEvent event;
        xQueueReceive(_eventQueue, &event, portMAX_DELAY);
        return event;
    }
}