#pragma once

#include "esp_event.h"

namespace scale::lcd::events {
    ESP_EVENT_DECLARE_BASE(LCD_EVENT);

    enum Event {
        EVENT_FORCE_REDRAW,
        EVENT_WEIGHT_UPDATED,
        EVENT_WEIGHT_STABILITY,
        EVENT_WIFI_CONNECTION_CHANGE,
        EVENT_MQTT_CONNECTION_CHANGE,
        EVENT_MAINTENANCE_CHANGE,
        EVENT_WEIGHT_REPORTED_BLINK,
        EVENT_UPDATE_STATE_CHANGED,
        EVENT_BACKLIGHT_STATE_CHANGED,
    };

    struct BaseEvent {
        static const Event event_id;
    };

    struct EventForceRedraw : BaseEvent {
        static const Event event_id = EVENT_FORCE_REDRAW;
    };
    struct EventWeightUpdated : BaseEvent {
        static const Event event_id = EVENT_WEIGHT_UPDATED;
        float grams;
    };
    struct EventScaleStability : BaseEvent {
        static const Event event_id = EVENT_WEIGHT_STABILITY;
        bool isStable;
    };
    struct EventWifiConnectionChange : BaseEvent {
        static const Event event_id = EVENT_WIFI_CONNECTION_CHANGE;
        bool isConnected;
    };
    struct EventMQTTConnectionChange : BaseEvent {
        static const Event event_id = EVENT_MQTT_CONNECTION_CHANGE;
        bool isConnected;
    };
    struct EventMaintenanceChange : BaseEvent {
        static const Event event_id = EVENT_MAINTENANCE_CHANGE;
        bool isMaintenanceModeOn;
    };
    struct EventWeightReportedBlink : BaseEvent {
        static const Event event_id = EVENT_WEIGHT_REPORTED_BLINK;
        bool blink;
    };
    struct EventUpdateStateChanged : BaseEvent {
        static const Event event_id = EVENT_UPDATE_STATE_CHANGED;
        bool isUpdateRunning;
        int updatePercentage;
    };
    struct EventBacklightStateChanged : BaseEvent {
        static const Event event_id = EVENT_BACKLIGHT_STATE_CHANGED;
        bool isBacklightOn;
    };
}