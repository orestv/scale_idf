#pragma once

#include "esp_err.h"
#include "esp_log.h"

#include <string>

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

namespace scale::persistence {
    const char *NAMESPACE = "storage";

    const char *TAG2 = "PersistenceUtil";
    
    template <typename T>
    esp_err_t save(const T &data, const std::string tag) {
        ESP_LOGI(TAG2, "Persisting data");
        nvs_handle_t nvs_handle;
        esp_err_t err;

        // Open
        err = nvs_open(NAMESPACE, NVS_READWRITE, &nvs_handle);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG2, "Error saving data: %d (%s)", err, esp_err_to_name(err));
            return err;
        }
        err = nvs_set_blob(nvs_handle, tag.c_str(), &data, sizeof(data));
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG2, "Error saving data: %d (%s)", err, esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(TAG2, "Data persisted");
        return ESP_OK;
    }

    template <typename T>
    esp_err_t load(T &data, const std::string tag) {
        ESP_LOGI(TAG2, "Loading data");
        nvs_handle_t nvs_handle;
        esp_err_t err;

        // Open
        err = nvs_open(NAMESPACE, NVS_READWRITE, &nvs_handle);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG2, "Error loading data: %d (%s)", err, esp_err_to_name(err));
            return err;
        }
        size_t required_size = sizeof(T);
        err = nvs_get_blob(nvs_handle, tag.c_str(), &data, &required_size);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG2, "Error loading data: %d (%s)", err, esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(TAG2, "Data loaded");
        return ESP_OK;
    }
}