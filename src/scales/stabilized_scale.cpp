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
            processEvent(incomingEvent);
        }        
    }

    void StabilizedScale::processEvent(const adapted::ScaleEvent &incomingEvent) {
        _stabilizer.push(incomingEvent.grams);
        if (!_stabilizer.isStable()) {
            // todo: send unstable scale event
            return;
        }
        float stabilizedValue = _stabilizer.getValue();
        ScaleEvent outgoingEvent = {
            .eventType=EVENT_WEIGHT,
            .grams=stabilizedValue,
        };
        xQueueSend(_eventQueue, &outgoingEvent, portMAX_DELAY);
    }

    ScaleEvent StabilizedScale::getEvent() {
        ScaleEvent event;
        xQueueReceive(_eventQueue, &event, portMAX_DELAY);
        return event;
    }
}