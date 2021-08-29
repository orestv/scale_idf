#include "scale/color_report.h"
#include "led.h"

#include "esp_log.h"

namespace scale::color {
    ColorReport::ColorReport(const ColorReportArgs &args): _led(args.led), _eventLoop(args.eventLoop) {
        esp_event_handler_register_with(
            _eventLoop,
            events::SCALE_EVENT,
            events::EVENT_RAW_TARED_WEIGHT_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                ESP_LOGD("ColorReport", "Received tared weight changed event");
                ColorReport &_this = *(ColorReport*)arg;
                events::EventRawTaredWeightChanged &evt = *(events::EventRawTaredWeightChanged*)event_data;
                _this.onWeightChanged(evt.grams);
            },
            this
        );
    }

    void ColorReport::onWeightChanged(float weight) {
        scale::led::Color red(255, 0, 0), green(0, 255, 0), yellow(255, 255, 0);

        scale::led::Color color;
        if (weight < 25) {
            color = red;
        } else if (weight < 45) {
            color = yellow;
        } else {
            color = green;
        }
        this->_led.setColor(color);
    }
}