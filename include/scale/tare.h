#pragma once

#include <vector>

#include "tare_persistence.h"

#include "scale/events.h"

namespace scale::tare {
    struct TareConfig {
        float zeroAtGrams;
    };

    class TareConfigBuilder {
    public:
        void reset();
        void push(float grams);
        bool isTareReady() const;
        TareConfig config() const;
    private:
        int MAX_BUFFER_SIZE = 1;

        std::vector<float> _tareBuffer;
    };

    class Tare {
    public:
        Tare(persistence::TarePersistence &persistence, esp_event_loop_handle_t eventLoop);

        void update(const TareConfig &config) {
            _taring = false;
            _config = config;
            persistence::TareSaveData saveData = {
                .tareGrams = config.zeroAtGrams,
            };
            _persistence.save(saveData);

            events::EventTareComplete evt = {};
            esp_event_post_to(
                _eventLoop,
                events::SCALE_EVENT,
                events::EVENT_TARE_COMPLETE,
                &evt,
                sizeof(evt),
                portMAX_DELAY
            );
        }
        float tare(float grams) {
            return grams - _config.zeroAtGrams;
        }
    private:
        void onRawWeightChanged(const events::EventRawWeightChanged &evt);
        void onStabilizedWeightChanged(const events::EventStabilizedWeightChanged &evt);
        void onTareRequested(const events::EventTareStarted &evt);

        bool _taring;

        TareConfig _config;
        persistence::TarePersistence &_persistence;
        esp_event_loop_handle_t _eventLoop;
    };
}