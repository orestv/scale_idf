#include "weight_converter.h"

namespace scale::raw {
    float GramConverter::grams(int raw_data) const {
        return (raw_data - _config.zero) * _config.coefficient;
    }
}