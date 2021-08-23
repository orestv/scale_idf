/* WiFi station Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi.h"
#include "led.h"
#include "color_report.h"
#include "mqtt.h"

#include "scale_button.h"

#define ESP_INTR_FLAG_DEFAULT 0

const gpio_num_t GPIO_PIN_RGB_RED = GPIO_NUM_32;
const gpio_num_t GPIO_PIN_RGB_GREEN = GPIO_NUM_25;
const gpio_num_t GPIO_PIN_RGB_BLUE = GPIO_NUM_26;

const gpio_num_t GPIO_PIN_BUTTON_TARE = GPIO_NUM_17;
const gpio_num_t GPIO_PIN_BUTTON_MAINTENANCE = GPIO_NUM_18;

extern "C" {
    void app_main(void);
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));

    scale::led::LEDPins ledPins{
        .gpio_red = GPIO_PIN_RGB_RED,
        .gpio_green = GPIO_PIN_RGB_GREEN,
        .gpio_blue = GPIO_PIN_RGB_BLUE,
    };
    scale::led::LED led(ledPins);
    scale::color::ColorReport color(led);
    scale::peri::button::PushButton buttonTare(
        {
            .buttonGPIO = GPIO_PIN_BUTTON_TARE,
        }
    );
    scale::peri::button::PushButton buttonMaintenance(
        {
            .buttonGPIO = GPIO_PIN_BUTTON_MAINTENANCE,
        }
    );

    float weight = 60;
    while (true) {

        color.onWeightChanged(weight);
        vTaskDelay(pdMS_TO_TICKS(30));

        weight -= 0.5;
        if (weight < 0) weight = 60;
    }
}