#pragma once

namespace scale::raw {

    struct GramConverterConfig {
        int zero;
        float coefficient;
    };

    class GramConverter {
    public:
        GramConverter(const GramConverterConfig &config): _config(config) {} 
        
        float grams(int raw_data) const;
    private:
        GramConverterConfig _config;
    };
}