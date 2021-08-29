#include "stabilized_scale.h"

#include "esp_log.h"

namespace scale::stabilized {

    void StabilizedScale::start() {
        esp_event_handler_register_with(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_RAW_WEIGHT_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                ESP_LOGI("Stabilized", "Processing event");
                StabilizedScale &_this = *(StabilizedScale*)arg;
                events::EventRawWeightChanged event = *(events::EventRawWeightChanged*)event_data;
                _this.processEvent(event);
                ESP_LOGI("Stabilized", "Event processed");
            }, 
            this
        );
            // xTaskCreate(
            //     [](void *arg)
            //     {
            //         ((StabilizedScale *)arg)->task();
            //     },
            //     "StabilizedScale", 2048, this, 10, nullptr);
    }

    // void StabilizedScale::task() {
        // while (true) {
            // scale::adapted::ScaleEvent incomingEvent = _adaptedScale.getEvent();
            // processEvent(incomingEvent);
        // }        
    // }

    void StabilizedScale::processEvent(const events::EventRawWeightChanged &incomingEvent) {
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