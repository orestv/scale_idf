#pragma once

#include <vector>

namespace scale::tare {
    struct TareConfig {
        float tareGrams = 0;
    };

    class TareConfigBuilder {
    public:
        void reset();
        void push(float grams);
        bool isTareReady() const;
        TareConfig config() const;
    private:
        int MAX_BUFFER_SIZE = 4;

        std::vector<float> _tareBuffer;
    };

    class Tare {
    public:
        Tare(const TareConfig &config): _config(config) {}
    
        void configure(const TareConfig &config) {
            _config = config;
        }
        float tare(float grams) {
            return grams - _config.tareGrams;
        }
    private:
        TareConfig _config;
    };
}