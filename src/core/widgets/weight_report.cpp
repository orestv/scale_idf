#include "scale/widgets/weight_report.h"

#include "esp_task.h"

namespace scale::lcd {
WeightReportWidget::WeightReportWidget(
    i2c_lcd1602_info_t *lcdInfo,
    esp_event_loop_handle_t eventLoop,
    esp_event_loop_handle_t lcdEventLoop)
    : BaseWidget(lcdInfo, eventLoop, lcdEventLoop) {
    _blipQueue = xQueueCreate(1, 0);
    esp_event_handler_register_with(
        eventLoop,
        scale::events::SCALE_EVENT,
        scale::events::EVENT_WEIGHT_REPORTED,
        [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
            auto &_this = *(WeightReportWidget *)arg;
            _this.blip();
        },
        this);

    xTaskCreate(
        [](void *arg) {
            auto &_this = *(WeightReportWidget *)arg;
            while (true) {
                if (xQueueReceive(_this._blipQueue, nullptr, 0)) {
                    _this.renderBlip();
                }
            }
        },
        "Blip", 1024, this, 5, nullptr);
}

void WeightReportWidget::blip() {
    xQueueSend(_blipQueue, nullptr, 0);
}

void WeightReportWidget::renderBlip() {
    i2c_lcd1602_move_cursor(lcdInfo(), 19, 3);
    i2c_lcd1602_write_char(lcdInfo(), 'X');
    vTaskDelay(pdMS_TO_TICKS(300));
    i2c_lcd1602_move_cursor(lcdInfo(), 19, 3);
    i2c_lcd1602_write_char(lcdInfo(), ' ');
}

void WeightReportWidget::render() {
}

}  // namespace scale::lcd