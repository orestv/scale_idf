#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"


namespace scale::led {
    struct LEDPins {
        gpio_num_t gpio_red;
        gpio_num_t gpio_green;
        gpio_num_t gpio_blue;
    };

    enum LedColor {
        LED_RED = 0,
        LED_GREEN = 1,
        LED_BLUE = 2
    };

    class LED {
        public:
            LED(const LEDPins &pins) : 
                    _pins(pins),
                    _channels{LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2} {
            }

            void start();
        private:
            void setLevel(LedColor, float level);
            int maxDuty() const;

            LEDPins _pins;

            ledc_channel_t _channels[3];
    };
}