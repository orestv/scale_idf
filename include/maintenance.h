#pragma once

#include "persistence.h"

namespace scale::maintenance {
    class Maintenance {
    public:
        Maintenance(): _maintenanceModeOn(false) {
            _load();
        }

        bool isMaintenanceModeOn() const {
            return _maintenanceModeOn;
        }
        void toggleMaintenanceMode() {
            _maintenanceModeOn = !_maintenanceModeOn;
            _save();
        }
    private:
        void _save() {
            persistence::save<bool>(_maintenanceModeOn, _nvsTag);
        }
        void _load() {
            persistence::load<bool>(_maintenanceModeOn, _nvsTag);
        }

        bool _maintenanceModeOn;

        const char *_nvsTag = "maintenance";
    };
}