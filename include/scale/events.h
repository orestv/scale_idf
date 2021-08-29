#pragma once

#include "esp_event.h"

namespace scale::events {
    ESP_EVENT_DECLARE_BASE(SCALE_EVENT);

    enum Event {
        EVENT_RAW_WEIGHT_CHANGED,
        EVENT_RAW_TARED_WEIGHT_CHANGED,
        EVENT_STABILIZED_WEIGHT_CHANGED,
        EVENT_STABILIZED_TARED_WEIGHT_CHANGED,

        EVENT_SCALE_UNSTABLE,

        EVENT_WIFI_CONNECTION_CHANGED,
        EVENT_MQTT_CONNECTION_CHANGED,

        EVENT_TARE_STARTED,
        EVENT_TARE_COMPLETE,

        EVENT_MAINTENANCE_MODE_ON,
        EVENT_MAINTENANCE_MODE_OFF,

        EVENT_BUTTON_TARE_PRESSED,
        EVENT_BUTTON_MAINTENANCE_PRESSED,
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

    struct EventScaleUnstable {};

    struct EventWifiConnectionChanged  {
        bool connected;
    };
    struct EventMQTTConnectionChanged {
        bool connected;
    };

    struct EventTareStarted {};
    struct EventTareComplete {};
}