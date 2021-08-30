#include "scale/widgets/wifi.h"

namespace scale::lcd {
WifiWidget::WifiWidget(
    i2c_lcd1602_info_t *lcdInfo,
    esp_event_loop_handle_t eventLoop,
    esp_event_loop_handle_t lcdEventLoop)
    : BaseWidget(lcdInfo, eventLoop, lcdEventLoop) {
    esp_event_handler_register_with(
        eventLoop,
        scale::events::SCALE_EVENT,
        scale::events::EVENT_WIFI_CONNECTION_CHANGED,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(WifiWidget *)arg;
            auto &evt = *(scale::events::EventWifiConnectionChanged *)event_data;
            _this._wifiConnected = evt.connected;
            events::EventWifiConnectionChange outgoingEvent;
            outgoingEvent.isConnected = _this._wifiConnected;
            _this.postEvent<events::EventWifiConnectionChange>(outgoingEvent);
        },
        this);

    esp_event_handler_register_with(
        lcdEventLoop, events::LCD_EVENT, events::EVENT_WIFI_CONNECTION_CHANGE,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(WifiWidget*)arg;
            _this.render();
        }, this
    );
}

void WifiWidget::render() {
    i2c_lcd1602_move_cursor(lcdInfo(), 15, 0);
    i2c_lcd1602_write_string(lcdInfo(), "WiFi");
    i2c_lcd1602_write_char(lcdInfo(), _wifiConnected ? '+' : '-');
}

}  // namespace scale::lcd