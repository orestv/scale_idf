#pragma once

namespace scale::persistence {
    struct TareSaveData {
        float tareGrams;
    };

    class TarePersistence {
    public:
        void save(const TareSaveData &data);
        TareSaveData load();
    };
}