#pragma once

#include "esp_event.h"

namespace scale::events {
    ESP_EVENT_DECLARE_BASE(SCALE_EVENT);

    enum Events {
        EVENT_RAW_WEIGHT_CHANGED,
        EVENT_RAW_TARED_WEIGHT_CHANGED,
        EVENT_STABILIZED_WEIGHT_CHANGED,
        EVENT_STABILIZED_TARED_WEIGHT_CHANGED,
        EVENT_WIFI_CONNECTION_CHANGED,
        EVENT_MQTT_CONNECTION_CHANGED,        
    };

    struct EventRawWeightChanged {
        float grams;
    };
    struct EventRawTaredWeightChanged {
        float grams;
    };
    struct EventStabilizedWeightChanged {
        float grams;
    };
    struct EventStabilizedTaredWeightChanged
    {
        float grams;
    };
}