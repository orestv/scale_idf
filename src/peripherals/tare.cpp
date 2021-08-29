#include "scale/tare.h"

#include "esp_log.h"

namespace scale::tare {
    const char *TAG = "Tare";
    Tare::Tare(persistence::TarePersistence &persistence, esp_event_loop_handle_t eventLoop) : _taring(false),
                                                                                           _persistence(persistence),
                                                                                           _eventLoop(eventLoop) {
        persistence::TareSaveData saveData = _persistence.load();
        TareConfig tareConfig = {
            .zeroAtGrams = saveData.tareGrams,
        };
        _config = tareConfig;

        esp_event_handler_register_with(
            eventLoop,
            events::SCALE_EVENT,
            events::EVENT_RAW_WEIGHT_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                Tare &_this = *(Tare *)arg;
                events::EventRawWeightChanged &evt = *(events::EventRawWeightChanged *)event_data;
                _this.onRawWeightChanged(evt);
            },
            this);
        esp_event_handler_register_with(
            eventLoop,
            events::SCALE_EVENT,
            events::EVENT_STABILIZED_WEIGHT_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                Tare &_this = *(Tare *)arg;
                events::EventStabilizedWeightChanged &evt = *(events::EventStabilizedWeightChanged *)event_data;
                _this.onStabilizedWeightChanged(evt);
            },
            this);
        esp_event_handler_register_with(
            eventLoop,
            events::SCALE_EVENT,
            events::EVENT_BUTTON_TARE_PRESSED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                Tare &_this = *(Tare *)arg;
                _this.onTareRequested();
            },
            this);
    }

    void Tare::onRawWeightChanged(const events::EventRawWeightChanged &evt) {
        ESP_LOGI(TAG, "Raw weight change event");
        if (_taring) {
            return;
        }
        float taredGrams = this->tare(evt.grams);
        events::EventRawTaredWeightChanged outgoingEvent = {
            .grams = taredGrams,
        };
        esp_event_post_to(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_RAW_TARED_WEIGHT_CHANGED,
            &outgoingEvent,
            sizeof(outgoingEvent),
            portMAX_DELAY
        );
    }

    void Tare::onStabilizedWeightChanged(const events::EventStabilizedWeightChanged &evt) {
        ESP_LOGI(TAG, "Stabilized weight change event: %f", evt.grams);
        if (_taring) {
            TareConfig tareConfig = {
                .zeroAtGrams = evt.grams
            };
            this->update(tareConfig);
        }
        float taredGrams = this->tare(evt.grams);
        ESP_LOGI(TAG, "Tared grams: %f", taredGrams);
        events::EventStabilizedTaredWeightChanged outgoingEvent = {
            .grams = taredGrams,
        };
        esp_event_post_to(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_STABILIZED_TARED_WEIGHT_CHANGED,
            &outgoingEvent,
            sizeof(outgoingEvent),
            portMAX_DELAY);
    }

    void Tare::onTareRequested() {
        _taring = true;
        esp_event_post_to(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_TARE_STARTED,
            nullptr,
            0,
            portMAX_DELAY
        );
    }

    void TareConfigBuilder::push(float grams) {
        _tareBuffer.push_back(grams);
    }

    bool TareConfigBuilder::isTareReady() const {
        return _tareBuffer.size() >= MAX_BUFFER_SIZE;
    }

    void TareConfigBuilder::reset() {
        _tareBuffer.clear();
    }

    TareConfig TareConfigBuilder::config() const {
        float sum = 0;
        for (const float &t : _tareBuffer) {
            sum += t;
        }
        float tare = sum / _tareBuffer.size();
        TareConfig result;
        result.zeroAtGrams = tare;
        return result;
    }
}