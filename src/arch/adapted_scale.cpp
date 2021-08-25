#include "adapted_scale.h"

#include "esp_log.h"

namespace scale::weight {
    void AdaptedScale::start() {
        xTaskCreate(AdaptedScale::processTask, "AdaptedScaleProcess", 2048, this, 10, nullptr);
    }

    void AdaptedScale::processTask(void *arg) {
        AdaptedScale &_this = *((AdaptedScale*)arg);

        while (true) {
            raw::ScaleEvent incomingEvent;

            if (xQueueReceive(_this._scale.queue(), &incomingEvent, portMAX_DELAY)) {
                _this.processEvent(incomingEvent);
            }
        }
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