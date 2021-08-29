#pragma once

#include "persistence.h"

#include "scale/events.h"

namespace scale::maintenance {
    class Maintenance {
    public:
        Maintenance(esp_event_loop_handle_t eventLoop): _maintenanceModeOn(false), _eventLoop(eventLoop) {
            _load();
            ESP_LOGI("Maintenance", "Maintenance mode is %s", _maintenanceModeOn ? "ON" : "OFF");

            esp_event_handler_register_with(
                eventLoop,
                events::SCALE_EVENT,
                events::EVENT_BUTTON_MAINTENANCE_PRESSED,
                [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                    ESP_LOGI("Maintenance", "Maintenance button press detected");
                    auto &_this = *(Maintenance*)arg;
                    _this.toggleMaintenanceMode();
                },
                this
            );
        }

        bool isMaintenanceModeOn() const {
            return _maintenanceModeOn;
        }
        void toggleMaintenanceMode() {
            _maintenanceModeOn = !_maintenanceModeOn;
            _save();
            _notifyMaintenanceMode();
        }
    private:
        void _notifyMaintenanceMode() {
            events::EventMaintenanceModeChanged event = {
                .isMaintenanceModeOn = _maintenanceModeOn,
            };
            esp_event_post_to(
                _eventLoop,
                events::SCALE_EVENT,
                events::EVENT_MAINTENANCE_MODE_CHANGED,
                &event,
                sizeof(event),
                portMAX_DELAY);
        }

        void _save() {
            persistence::save<bool>(_maintenanceModeOn, _nvsTag);
        }
        void _load() {
            persistence::load<bool>(_maintenanceModeOn, _nvsTag);
            _notifyMaintenanceMode();
        }

        bool _maintenanceModeOn;
        esp_event_loop_handle_t _eventLoop;

        const char *_nvsTag = "maintenance";
    };
}