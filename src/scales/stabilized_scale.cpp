#include "stabilized_scale.h"

#include "esp_log.h"

namespace scale::stabilized {

    const char *TAG = "StabilizedScale";

    StabilizedScale::StabilizedScale(Stabilizer &stabilizer, esp_event_loop_handle_t eventLoop) : _eventLoop(eventLoop),
                                                                                 _stabilizer(stabilizer) {
        _eventQueue = xQueueCreate(10, sizeof(ScaleEvent));
        start();
    }

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
        ESP_LOGI(TAG, "Received adapted data %f, stabilizing", incomingEvent.grams);
        _stabilizer.push(incomingEvent.grams);
        if (!_stabilizer.isStable()) {
            // todo: send "unstable" event
            return;
        }
        // todo: send "stable" event   
        float stabilizedValue = _stabilizer.getValue();
        events::EventStabilizedWeightChanged evt = {
            .grams = stabilizedValue,
        };

        ESP_LOGI(TAG, "Sending stabilized value %f", stabilizedValue);

        esp_event_post_to(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_STABILIZED_WEIGHT_CHANGED,
            &evt,
            sizeof(evt),
            portMAX_DELAY
        );
    }
}