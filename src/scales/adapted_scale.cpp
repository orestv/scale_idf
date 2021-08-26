#include "adapted_scale.h"

#include "esp_log.h"

namespace scale::adapted {
    void AdaptedScale::start() {
        xTaskCreate(
            [](void *arg) {((AdaptedScale*)arg)->task();},
            "AdaptedScaleProcess", 2048, this, 10, nullptr);
    }

    void AdaptedScale::task() {
        while (true) {
            raw::ScaleEvent incomingEvent = _scale.getEvent();
            processEvent(incomingEvent);
        }
    }

    ScaleEvent AdaptedScale::getEvent() {
        ScaleEvent event;
        xQueueReceive(this->_eventQueue, &event, portMAX_DELAY);
        return event;
    }

    void AdaptedScale::processEvent(const raw::ScaleEvent &incomingEvent) {
        ScaleEvent outgoingEvent = this->convertEvent(incomingEvent);
        xQueueSend(this->_eventQueue, &outgoingEvent, portMAX_DELAY);
    }

    ScaleEvent AdaptedScale::convertEvent(const raw::ScaleEvent &incomingEvent) const {
        float grams = this->_converter.grams(incomingEvent.rawData);
        return {
            .grams = grams,
        };
    }
}