#include "scale/widgets/maintenance.h"

#include "scale/events.h"

#include <cstring>
#include <sstream>
#include <iomanip>

namespace scale::lcd {
MaintenanceWidget::MaintenanceWidget(
    i2c_lcd1602_info_t *lcdInfo,
    esp_event_loop_handle_t eventLoop) : BaseWidget(lcdInfo, eventLoop) {

        _maintenanceOn = false;

        esp_event_handler_register_with(
            eventLoop,
            events::SCALE_EVENT,
            events::EVENT_MAINTENANCE_MODE_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(MaintenanceWidget *)arg;
                auto &evt = *(events::EventMaintenanceModeChanged *)event_data;
                _this._maintenanceOn = evt.isMaintenanceModeOn;
                _this.render();
            },
            this);
}

void MaintenanceWidget::render() {
    const char *sMaintenance = _maintenanceOn ? "MAINTENANCE" : "           ";
    i2c_lcd1602_move_cursor(lcdInfo(), 0, 0);
    i2c_lcd1602_write_string(lcdInfo(), sMaintenance);
}
}  // namespace scale::lcd