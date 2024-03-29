#include "scale/widgets/maintenance.h"

#include <cstring>
#include <iomanip>
#include <sstream>

#include "scale/events.h"
#include "scale/widgets/events.h"

namespace scale::lcd {
MaintenanceWidget::MaintenanceWidget(
    i2c_lcd1602_info_t *lcdInfo,
    esp_event_loop_handle_t eventLoop,
    esp_event_loop_handle_t lcdEventLoop)
    : BaseWidget(lcdInfo, eventLoop, lcdEventLoop) {
    _maintenanceOn = false;

    esp_event_handler_register_with(
        eventLoop,
        scale::events::SCALE_EVENT,
        scale::events::EVENT_MAINTENANCE_MODE_CHANGED,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(MaintenanceWidget *)arg;
            auto &evt = *(scale::events::EventMaintenanceModeChanged *)event_data;
            _this._maintenanceOn = evt.isMaintenanceModeOn;
            events::EventMaintenanceChange internalEvent;
            internalEvent.isMaintenanceModeOn = _this._maintenanceOn;
            _this.postEvent<events::EventMaintenanceChange>(internalEvent);
        },
        this);
    
    esp_event_handler_register_with(
        lcdEventLoop,
        events::LCD_EVENT,
        events::EVENT_MAINTENANCE_CHANGE,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(MaintenanceWidget*)arg;
            auto &event = *(events::EventMaintenanceChange*)event_data;
            _this.render();
        }, this
    );
}

void MaintenanceWidget::render() {
    const char *sMaintenance = _maintenanceOn ? "MAINTENANCE" : "             ";
    i2c_lcd1602_move_cursor(lcdInfo(), 0, 0);
    i2c_lcd1602_write_string(lcdInfo(), sMaintenance);
}
}  // namespace scale::lcd