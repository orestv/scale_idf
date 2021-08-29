#include "scale/widgets/wifi.h"

namespace scale::lcd {
WifiWidget::WifiWidget(
    i2c_lcd1602_info_t *lcdInfo,
    esp_event_loop_handle_t eventLoop)
    : _lcdInfo(lcdInfo),
      _eventLoop(eventLoop) {
          esp_event_handler_register_with(
              eventLoop,
              events::SCALE_EVENT,
              events::EVENT_WIFI_CONNECTION_CHANGED,
              [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                  auto &_this = *(WifiWidget*)arg;
                  auto &evt = *(events::EventWifiConnectionChanged*)event_data;
                  _this._wifiConnected = evt.connected;
                  _this.render();
              },
              this
          );
}

void WifiWidget::render() {
    i2c_lcd1602_move_cursor(_lcdInfo, 15, 0);
    i2c_lcd1602_write_string(_lcdInfo, "WiFi");
    i2c_lcd1602_move_cursor(_lcdInfo, 19, 0);
    i2c_lcd1602_write_char(_lcdInfo, _wifiConnected ? '+' : '-');

}

}  // namespace scale::lcd