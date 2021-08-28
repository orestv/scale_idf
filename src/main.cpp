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

#include "raw_scale.h"
#include "weight_converter.h"

#include "stabilizer.h"
#include "stabilized_scale.h"
#include "adapted_scale.h"
#include "tare_persistence.h"
#include "tare_controller.h"

#include "wifi.h"
#include "mqtt.h"
#include "mqtt_report.h"

#include "maintenance.h"

#include "controller.h"

#define ESP_INTR_FLAG_DEFAULT 0

const std::string AP_HOSTNAME = "feeder";
const std::string AP_SSID = "Rift2.4";
const std::string AP_WPA2_PWD = "breakdown";

const std::string MQTT_BROKER_URL = "mqtt://openhab";
const std::string MQTT_TOPIC_WEIGHT = "/feeder/weight";
const std::string MQTT_TOPIC_STABLE = "/feeder/stable";

const gpio_num_t GPIO_RGB_RED = GPIO_NUM_32;
const gpio_num_t GPIO_RGB_GREEN = GPIO_NUM_25;
const gpio_num_t GPIO_RGB_BLUE = GPIO_NUM_26;

const gpio_num_t GPIO_BUTTON_TARE = GPIO_NUM_13;
const gpio_num_t GPIO_BUTTON_MAINTENANCE = GPIO_NUM_15;

const gpio_num_t GPIO_HX711_CLK = GPIO_NUM_21;
const gpio_num_t GPIO_HX711_DAT = GPIO_NUM_22;

extern "C" {
    void app_main(void);
}

void init_esp() {

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
}

void app_main(void)
{
    init_esp();

    scale::led::LEDPins ledPins{
        .gpio_red = GPIO_RGB_RED,
        .gpio_green = GPIO_RGB_GREEN,
        .gpio_blue = GPIO_RGB_BLUE,
    };
    scale::led::LED led(ledPins);
    scale::color::ColorReport colorReport(led);
    scale::peri::button::PushButton buttonTare(
        {
            .buttonGPIO = GPIO_BUTTON_TARE,
        }
    );
    scale::peri::button::PushButton buttonMaintenance(
        {.buttonGPIO = GPIO_BUTTON_MAINTENANCE,}
    );

    scale::raw::Scale rawScale(
        {
            .gpioDAT = GPIO_HX711_DAT,
            .gpioCLK = GPIO_HX711_CLK,
        }
    );

    scale::raw::GramConverter converter(
        {
            .zero = 0,
            .coefficient = 0.0018,
        }
    );

    scale::stabilized::Stabilizer stabilizer({.dataPoints=5, .margin=0.5});
    
    scale::adapted::AdaptedScale adaptedScale(rawScale, converter);
    scale::stabilized::StabilizedScale stabilizedScale(adaptedScale, stabilizer);

    scale::persistence::TarePersistence tarePersistence;
    
    scale::tare::TareConfigBuilder tareConfigBuilder;
    scale::tare::Tare tare(tarePersistence);

    scale::wifi::WifiClient wifiClient({.hostname=AP_HOSTNAME});
    wifiClient.start(
        {
            .ssid=AP_SSID,
            .wpa2Password=AP_WPA2_PWD,
        }, 1
    );

    scale::mqtt::MQTTClient mqttClient(
        {.brokerUrl=MQTT_BROKER_URL}, {}
    );

    scale::mqtt::MQTTReportConfig reportConfig = {
        .topics = {
            .topicWeight=MQTT_TOPIC_WEIGHT,
            .topicStable=MQTT_TOPIC_STABLE,
        },
        .mqttClient=mqttClient,
    };
    scale::mqtt::MQTTReport mqttReport(reportConfig);

    stabilizedScale.start();

    scale::maintenance::Maintenance maintenance;

    scale::controller::ScaleControllerArgs args = {
        .buttonTare = buttonTare,
        .buttonMaintenance = buttonMaintenance,
        .maintenance = maintenance,
        .colorReport = colorReport,
        .stabilizedScale = stabilizedScale,
        .tare=tare,
        .tareConfigBuilder=tareConfigBuilder,
        .mqttReport=mqttReport,
    };
    scale::controller::ScaleController controller(args);
    
    controller.start();

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}