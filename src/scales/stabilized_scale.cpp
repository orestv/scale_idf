#include "stabilized_scale.h"

#include "esp_log.h"

namespace scale::stabilized {

    void StabilizedScale::start() {
        esp_event_handler_register_with(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_RAW_WEIGHT_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                StabilizedScale &_this = *(StabilizedScale*)arg;
                events::EventRawWeightChanged event = *(events::EventRawWeightChanged*)event_data;
                _this.processEvent(event);
            }, 
            this
        );
    }

    void StabilizedScale::processEvent(const events::EventRawWeightChanged &incomingEvent) {
        _stabilizer.push(incomingEvent.grams);
        if (!_stabilizer.isStable()) {
            // todo: send "unstable" event
            ScaleEvent outgoingEvent = {
                .eventType=EVENT_UNSTABLE,
                .grams=0,
                .gramsRaw=incomingEvent.grams,
            };
            xQueueSend(_eventQueue, &outgoingEvent, portMAX_DELAY);
            return;
        }
        // todo: send "stable" event;        
        float stabilizedValue = _stabilizer.getValue();
        events::EventStabilizedWeightChanged evt = {
            .grams = stabilizedValue,
        };

        esp_event_post_to(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_STABILIZED_WEIGHT_CHANGED,
            &evt,
            sizeof(evt),
            portMAX_DELAY
        );
        // ScaleEvent outgoingEvent = {
        //     .eventType=EVENT_STABLE_WEIGHT,
        //     .grams=stabilizedValue,
        //     .gramsRaw=incomingEvent.grams,
        // };
        // xQueueSend(_eventQueue, &outgoingEvent, portMAX_DELAY);
    }

    ScaleEvent StabilizedScale::getEvent() {
        ScaleEvent event;
        xQueueReceive(_eventQueue, &event, portMAX_DELAY);
        return event;
    }
}