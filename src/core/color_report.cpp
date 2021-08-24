#include "color_report.h"
#include "led.h"

namespace scale::color {
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