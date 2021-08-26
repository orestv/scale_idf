#include "tare_persistence.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

#define STORAGE_NAMESPACE "storage"

namespace scale::persistence {
    const char *TAG = "TarePersistence";

    const char *NVS_TAG = "tare";

    void TarePersistence::save(const TareSaveData &data) {
        ESP_LOGI(TAG, "Persisting tare");
        nvs_handle_t nvs_handle;
        esp_err_t err;

        // Open
        err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error saving tare: %d (%s)", err, esp_err_to_name(err));
            return;
        }
        size_t required_size = sizeof(data);
        err = nvs_set_blob(nvs_handle, NVS_TAG, &data, sizeof(data));
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error saving tare: %d (%s)", err, esp_err_to_name(err));
            return;
        }
        ESP_LOGI(TAG, "Tare persisted");
    }

    TareSaveData TarePersistence::load() {
        TareSaveData result = {.tareGrams=0};
        ESP_LOGI(TAG, "Loading tare");
        nvs_handle_t nvs_handle;
        esp_err_t err;

        // Open
        err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
        err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error loading tare: %d (%s)", err, esp_err_to_name(err));
            return result;
        }
        size_t required_size = sizeof(TareSaveData);
        err = nvs_get_blob(nvs_handle, NVS_TAG, &result, &required_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error loading tare: %d (%s)", err, esp_err_to_name(err));
            return result;
        }
        ESP_LOGI(TAG, "Tare loaded");
        return result;
    }
}