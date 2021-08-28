#include "tare_persistence.h"
#include "persistence.h"

#include "esp_log.h"

#define STORAGE_NAMESPACE "storage"

namespace scale::persistence {
    const char *TAG = "TarePersistence";

    void TarePersistence::save(const TareSaveData &data) {
        ESP_LOGI(TAG, "Persisting tare");
        esp_err_t err = persistence::save<TareSaveData>(data);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error saving tare: %d (%s)", err, esp_err_to_name(err));
        }
        ESP_LOGI(TAG, "Tare persisted");
    }

    TareSaveData TarePersistence::load() {
        TareSaveData result = {.tareGrams=0};
        esp_err_t err = persistence::load<TareSaveData>(result);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error loading tare: %d (%s)", err, esp_err_to_name(err));
        }
        return result;
    }
}