#include "scale/widgets/update.h"

#include "scale/events.h"

#include <sstream>

namespace scale::lcd {
UpdateWidget::UpdateWidget(
    i2c_lcd1602_info_t *lcdInfo,
    esp_event_loop_handle_t eventLoop,
    esp_event_loop_handle_t lcdEventLoop) : BaseWidget(lcdInfo, eventLoop, lcdEventLoop) {

        esp_event_handler_register_with(
            eventLoop, scale::events::SCALE_EVENT,
            scale::events::EVENT_UPDATE_STATE_CHANGE,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(UpdateWidget*)arg;
                auto &scaleEvent = *(scale::events::EventUpdateStateChange*)event_data;
                events::EventUpdateStateChanged lcdEvent;
                lcdEvent.isUpdateRunning = scaleEvent.isUpdating;
                lcdEvent.updatePercentage = scaleEvent.updatePercentage;
                esp_event_post_to(
                    _this.lcdEventLoop(),
                    events::LCD_EVENT,
                    events::EVENT_UPDATE_STATE_CHANGED,
                    &lcdEvent,
                    sizeof(lcdEvent), 0
                );
            }, this
        );

        esp_event_handler_register_with(
            lcdEventLoop, events::LCD_EVENT, events::EVENT_UPDATE_STATE_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(UpdateWidget *)arg;
                auto &event = *(events::EventUpdateStateChanged *)event_data;
                _this._lastEvent = event;
                _this.render();
            },
            this);
}

void UpdateWidget::render() {
    i2c_lcd1602_move_cursor(lcdInfo(), 0, 1);
    if (!_lastEvent.isUpdateRunning) {
        i2c_lcd1602_write_string(lcdInfo(), "              ");
        return;
    }

    std::ostringstream oss;
    oss << "Updating: " << _lastEvent.updatePercentage << "%";
    i2c_lcd1602_write_string(lcdInfo(), oss.str().c_str());
}
}  // namespace scale::lcd