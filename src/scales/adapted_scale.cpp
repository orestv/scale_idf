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
            ESP_LOGD("AdaptedScale", "Awaiting incoming event");
            raw::ScaleEvent incomingEvent = _scale.getEvent();
            ESP_LOGD("AdaptedScale", "Incoming event received, processing");
            processEvent(incomingEvent);
        }
    }

    void AdaptedScale::processEvent(const raw::ScaleEvent &incomingEvent) {
        ESP_LOGD("AdaptedScale", "Sending adapted scale event");
        events::EventRawWeightChanged outgoingEvent = this->convertEvent(incomingEvent);
        esp_event_post_to(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_RAW_WEIGHT_CHANGED,
            &outgoingEvent,
            sizeof(outgoingEvent), portMAX_DELAY
        );
    }

    events::EventRawWeightChanged AdaptedScale::convertEvent(const raw::ScaleEvent &incomingEvent) const {
        float grams = this->_converter.grams(incomingEvent.rawData);
        return {
            .grams = grams,
        };
    }
}