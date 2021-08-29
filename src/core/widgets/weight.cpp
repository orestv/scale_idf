#include "scale/widgets/weight.h"

#include "scale/events.h"

#include <cstring>
#include <sstream>
#include <iomanip>

namespace scale::lcd {
WeightWidget::WeightWidget(
    i2c_lcd1602_info_t *lcdInfo,
    esp_event_loop_handle_t eventLoop) : BaseWidget(lcdInfo, eventLoop) {

        _grams = 0;

        esp_event_handler_register_with(
            eventLoop,
            events::SCALE_EVENT,
            events::EVENT_STABILIZED_TARED_WEIGHT_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(WeightWidget *)arg;
                auto &evt = *(events::EventStabilizedTaredWeightChanged *)event_data;
                _this._grams = evt.grams;
                _this.render();
            },
            this);
}

void WeightWidget::render() {
    std::stringstream oss;
    oss << std::fixed 
        << std::setprecision(1) 
        << std::setw( 5 )
        << std::setfill(' ')
        <<  _grams;

    const char *strWeight = "Weight: ";
    i2c_lcd1602_move_cursor(lcdInfo(), 0, 3);
    i2c_lcd1602_write_string(lcdInfo(), strWeight);
    i2c_lcd1602_move_cursor(lcdInfo(), strlen(strWeight), 3);
    i2c_lcd1602_write_string(lcdInfo(), oss.str().c_str());
}
}  // namespace scale::lcd