#include "scale/widgets/mqtt.h"

namespace scale::lcd {
MQTTWidget::MQTTWidget(
    i2c_lcd1602_info_t *lcdInfo,
    esp_event_loop_handle_t eventLoop)
    : _lcdInfo(lcdInfo),
      _eventLoop(eventLoop) {
    esp_event_handler_register_with(
        eventLoop,
        events::SCALE_EVENT,
        events::EVENT_MQTT_CONNECTION_CHANGED,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(MQTTWidget *)arg;
            auto &evt = *(events::EventMQTTConnectionChanged *)event_data;
            _this._mqttConnected = evt.connected;
            _this.render();
        },
        this);
}

void MQTTWidget::render() {
    i2c_lcd1602_move_cursor(_lcdInfo, 15, 1);
    i2c_lcd1602_write_string(_lcdInfo, "MQTT");
    i2c_lcd1602_move_cursor(_lcdInfo, 19, 1);
    i2c_lcd1602_write_char(_lcdInfo, _mqttConnected ? '+' : '-');
}

}  // namespace scale::lcd