#include "led.h"

#include <stdio.h>
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#include <math.h>
#include <array>
#include <limits>

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz


namespace scale::led {
    const char *TAG = "LED";

    void LED::start() {
        ESP_LOGI(TAG, "Started");

        gpio_num_t pins[] = {_pins.gpio_red, _pins.gpio_green, _pins.gpio_blue};

        ledc_timer_config_t ledc_timer = {
            .speed_mode       = LEDC_MODE,
            .duty_resolution  = LEDC_DUTY_RES,
            .timer_num        = LEDC_TIMER,
            .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
            .clk_cfg          = LEDC_AUTO_CLK
        };
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
        
        for  (int i = 0; i < 3; ++i) {
            auto pin = pins[i];
            auto channel = _channels[i];

            // Prepare and then apply the LEDC PWM channel configuration
            ledc_channel_config_t ledc_channel = {
                .gpio_num       = pin,
                .speed_mode     = LEDC_MODE,
                .channel        = channel,
                .intr_type      = LEDC_INTR_DISABLE,
                .timer_sel      = LEDC_TIMER,
                // .intr_type      = LEDC_INTR_DISABLE,
                .duty           = 0, // Set duty to 0%
                .hpoint         = 0
            };
            ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
        }

        setLevel(LED_RED, 0.4);
        setLevel(LED_GREEN, 0);
        setLevel(LED_BLUE, 0);
    }

    void LED::setColor(const Color &color) {
        std::array<uint8_t, 3> components = {color.red, color.green, color.blue};
        auto maxComponentValue = std::numeric_limits<uint8_t>::max();
        std::array<LedColor, 3> colors = {LED_RED, LED_GREEN, LED_BLUE};

        for (int i = 0; i < components.size(); ++i) {
            auto level = (float)components[i] / maxComponentValue;
            auto color = colors[i];
            setLevel(color, level);
        }
    }

    void LED::setColor(uint8_t red, uint8_t green, uint8_t blue) {
        Color color = {
            .red = red,
            .green = green,
            .blue = blue
        };
        this->setColor(color);
    }

    void LED::setLevel(LedColor color, float level) {
        if (level < 0) level = 0;
        if (level > 1) level = 1;

        int duty = (int)floor(maxDuty() * level);
        ledc_channel_t channel = _channels[color];
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, channel, duty));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, channel));
    }

    int LED::maxDuty() const {
        return 1 << LEDC_DUTY_RES;
    }
}