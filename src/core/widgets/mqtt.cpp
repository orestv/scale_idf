#include "scale/widgets/mqtt.h"

namespace scale::lcd {
MQTTWidget::MQTTWidget(
    i2c_lcd1602_info_t *lcdInfo,
    esp_event_loop_handle_t eventLoop,
    esp_event_loop_handle_t lcdEventLoop)
    : BaseWidget(lcdInfo, eventLoop, lcdEventLoop) {
    esp_event_handler_register_with(
        eventLoop,
        scale::events::SCALE_EVENT,
        scale::events::EVENT_MQTT_CONNECTION_CHANGED,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(MQTTWidget *)arg;
            auto &evt = *(scale::events::EventMQTTConnectionChanged *)event_data;
            _this._mqttConnected = evt.connected;
            events::EventMQTTConnectionChange outgoingEvent;
            outgoingEvent.isConnected = _this._mqttConnected;
            _this.postEvent<events::EventMQTTConnectionChange>(outgoingEvent);
        },
        this);

    esp_event_handler_register_with(
        lcdEventLoop, events::LCD_EVENT, events::EVENT_MQTT_CONNECTION_CHANGE,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(MQTTWidget*)arg;
            _this.render();
        }, this
    );
}

void MQTTWidget::render() {
    i2c_lcd1602_move_cursor(lcdInfo(), 15, 1);
    i2c_lcd1602_write_string(lcdInfo(), "MQTT");
    i2c_lcd1602_write_char(lcdInfo(), _mqttConnected ? '+' : '-');
}

}  // namespace scale::lcd