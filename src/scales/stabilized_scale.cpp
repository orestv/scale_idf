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
            ScaleEvent outgoingEvent = {
                .eventType=EVENT_UNSTABLE,
                .grams=0,
                .gramsRaw=incomingEvent.grams,
            };
            xQueueSend(_eventQueue, &outgoingEvent, portMAX_DELAY);
            return;
        }
        float stabilizedValue = _stabilizer.getValue();
        ScaleEvent outgoingEvent = {
            .eventType=EVENT_STABLE_WEIGHT,
            .grams=stabilizedValue,
            .gramsRaw=incomingEvent.grams,
        };
        xQueueSend(_eventQueue, &outgoingEvent, portMAX_DELAY);
    }

    ScaleEvent StabilizedScale::getEvent() {
        ScaleEvent event;
        xQueueReceive(_eventQueue, &event, portMAX_DELAY);
        return event;
    }
}