#pragma once

#include <vector>

#include "tare_persistence.h"

#include "scale/events.h"

namespace scale::tare {
    struct TareConfig {
        float tareGrams;
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
        Tare(persistence::TarePersistence &persistence): _persistence(persistence) {
            persistence::TareSaveData saveData = _persistence.load();
            TareConfig tareConfig = {
                .tareGrams=saveData.tareGrams,
            };
            _config = tareConfig; 
        }
    
        void update(const TareConfig &config) {
            _config = config;
            persistence::TareSaveData saveData = {
                .tareGrams = config.tareGrams,
            };
            _persistence.save(saveData);
        }
        float tare(float grams) {
            return grams - _config.tareGrams;
        }
    private:
        TareConfig _config;
        persistence::TarePersistence &_persistence;
    };
}