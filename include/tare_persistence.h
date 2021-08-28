#pragma once

#include <string>

namespace scale::persistence {
    struct TareSaveData {
        float tareGrams;
    };

    class TarePersistence {
    public:
        void save(const TareSaveData &data);
        TareSaveData load();

        std::string _nvsTag = "tare";
    };
}