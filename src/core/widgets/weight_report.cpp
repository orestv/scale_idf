#include "scale/widgets/weight_report.h"

#include "esp_task.h"

namespace scale::lcd {
    const int BLINK_DURATION_MS = 350;
    // const char BLINK_CHARACTER = 'X';
    
    
    const char BLINK_CHARACTER = I2C_LCD1602_CHARACTER_CUSTOM_0;
    const uint8_t BLINK_CHARACTER_CUSTOM[8] = {
        0b00000,
        0b01010,
        0b11111,
        0b11111,
        0b01110,
        0b00100,
        0b00000,
    };

    WeightReportWidget::WeightReportWidget(i2c_lcd1602_info_t * lcdInfo, esp_event_loop_handle_t eventLoop, esp_event_loop_handle_t lcdEventLoop) : BaseWidget(lcdInfo, eventLoop, lcdEventLoop) {
        i2c_lcd1602_define_char(lcdInfo, I2C_LCD1602_INDEX_CUSTOM_0, BLINK_CHARACTER_CUSTOM);
        _blink = false;
        _blipQueue = xQueueCreate(1, 0);
        esp_event_handler_register_with(
            eventLoop,
            scale::events::SCALE_EVENT,
            scale::events::EVENT_WEIGHT_REPORTED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(WeightReportWidget *)arg;
                _this.scheduleBlink(true);
            },
            this);
        xTaskCreate(
            [](void *arg) {
                auto &_this = *(WeightReportWidget *)arg;
                while (true) {
                    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                    vTaskDelay(pdMS_TO_TICKS(BLINK_DURATION_MS));
                    _this.scheduleBlink(false);
                }
            },
            "Blip", 1024, this, 5, &_blipTask);

        esp_event_handler_register_with(
            lcdEventLoop, events::LCD_EVENT, events::EVENT_WEIGHT_REPORTED_BLINK,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(WeightReportWidget *)arg;
                auto &evt = *(events::EventWeightReportedBlink *)event_data;
                _this._blink = evt.blink;
                _this.render();
                if (_this._blink) {
                    xTaskNotifyGive(_this._blipTask);
                }
            },
            this);
    }

void WeightReportWidget::scheduleBlink(bool blip) {
    events::EventWeightReportedBlink outgoingEvent;
    outgoingEvent.blink = blip;
    postEvent<events::EventWeightReportedBlink>(outgoingEvent);
}

void WeightReportWidget::render() {
    const char blip = _blink ? BLINK_CHARACTER : ' ';
    i2c_lcd1602_move_cursor(lcdInfo(), 19, 3);
    i2c_lcd1602_write_char(lcdInfo(), blip);
}

}  // namespace scale::lcd