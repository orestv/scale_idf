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

#include "esp_sntp.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "scale/wifi.h"
#include "led.h"
#include "scale/color_report.h"
#include "mqtt_report.h"
#include "scale/mqtt.h"

#include "scale_button.h"

#include "raw_scale.h"
#include "weight_converter.h"

#include "stabilizer.h"
#include "stabilized_scale.h"
#include "adapted_scale.h"
#include "tare_persistence.h"
#include "scale/tare.h"

#include "maintenance.h"

#include "scale/events.h"

#include "scale/ota.h"
#include "scale/presence.h"
#include "scale/lcd.h"

#define ESP_INTR_FLAG_DEFAULT 0

const std::string AP_HOSTNAME = "feeder";
const std::string AP_SSID = "Rift2.4";
const std::string AP_WPA2_PWD = "breakdown";

const std::string MQTT_BROKER_URL = "mqtt://openhab";
const std::string MQTT_TOPIC_WEIGHT = "/feeder/weight";
const std::string MQTT_TOPIC_STABLE = "/feeder/stable";

const gpio_num_t GPIO_RGB_RED = GPIO_NUM_25;
const gpio_num_t GPIO_RGB_GREEN = GPIO_NUM_26;
const gpio_num_t GPIO_RGB_BLUE = GPIO_NUM_27;

const gpio_num_t GPIO_BUTTON_TARE = GPIO_NUM_13;
const gpio_num_t GPIO_BUTTON_MAINTENANCE = GPIO_NUM_15;

const gpio_num_t GPIO_HX711_CLK = GPIO_NUM_19;
const gpio_num_t GPIO_HX711_DAT = GPIO_NUM_18;

const gpio_num_t GPIO_LCD_SDA = GPIO_NUM_16;
const gpio_num_t GPIO_LCD_SCL = GPIO_NUM_17;

const gpio_num_t GPIO_PRESENCE = GPIO_NUM_33;

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
    esp_wifi_set_ps(WIFI_PS_NONE);

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void app_main(void)
{
    init_esp();

    esp_event_loop_args_t eventLoopArgs = {
        .queue_size=40,
        .task_name="ScaleEvents",
        .task_priority=5,
        .task_stack_size=4096,
    };
    esp_event_loop_handle_t scaleEventLoop;
    esp_event_loop_create(&eventLoopArgs, &scaleEventLoop);

    scale::lcd::LCDConfig lcdConfig = {
        .gpioSDA = GPIO_LCD_SDA,
        .gpioSCL = GPIO_LCD_SCL,
        .eventLoop = scaleEventLoop,
    };
    scale::lcd::LCD lcd(lcdConfig);
    lcd.start();
    lcd.waitUntilReady();

    scale::led::LEDPins ledPins{
        .gpio_red = GPIO_RGB_RED,
        .gpio_green = GPIO_RGB_GREEN,
        .gpio_blue = GPIO_RGB_BLUE,
    };
    scale::led::LED led(ledPins);

    scale::color::ColorReportArgs colorReportArgs = {
        .led = led,
        .eventLoop = scaleEventLoop,
    };
    scale::color::ColorReport colorReport(colorReportArgs);
    scale::peri::button::PushButton buttonTare(
        {
            .buttonGPIO = GPIO_BUTTON_TARE,
            .eventLoop = scaleEventLoop,
            .eventID = scale::events::EVENT_BUTTON_TARE_PRESSED,
        }
    );
    scale::peri::button::PushButton buttonMaintenance(
        {
            .buttonGPIO = GPIO_BUTTON_MAINTENANCE,
            .eventLoop = scaleEventLoop,
            .eventID = scale::events::EVENT_BUTTON_MAINTENANCE_PRESSED,
        }
    );

    scale::raw::Scale rawScale(
        {
            .gpioDAT = GPIO_HX711_DAT,
            .gpioCLK = GPIO_HX711_CLK,
            .eventLoop = scaleEventLoop,
        }
    );

    scale::raw::GramConverter converter(
        {
            .zero = 0,
            .coefficient = 0.0018,
        }
    );

    scale::stabilized::Stabilizer stabilizer({.dataPoints=20, .margin=3});
    
    scale::adapted::AdaptedScale adaptedScale(rawScale, converter, scaleEventLoop);
    scale::stabilized::StabilizedScale stabilizedScale(stabilizer, scaleEventLoop);

    scale::persistence::TarePersistence tarePersistence;
    
    scale::tare::TareConfigBuilder tareConfigBuilder;
    scale::tare::Tare tare(tarePersistence, scaleEventLoop);

    scale::wifi::WifiClient wifiClient({.hostname=AP_HOSTNAME, .eventLoop=scaleEventLoop});
    wifiClient.start(
        {
            .ssid=AP_SSID,
            .wpa2Password=AP_WPA2_PWD,
        }, 1
    );

    scale::mqtt::MQTTClient mqttClient(
        {.brokerUrl=MQTT_BROKER_URL, .eventLoop=scaleEventLoop}, {}
    );
    mqttClient.start();
    
    scale::maintenance::Maintenance maintenance(scaleEventLoop);

    scale::mqtt::MQTTReportConfig reportConfig = {
        .topics = {
            .topicWeight = MQTT_TOPIC_WEIGHT,
            .topicStable = MQTT_TOPIC_STABLE,
        },
        .mqttClient = mqttClient,
        .maintenance = maintenance,
        .eventLoop = scaleEventLoop,
    };
    scale::mqtt::MQTTReport mqttReport(reportConfig);

    scale::ota::OTAConfig otaConfig = {
        .eventLoop = scaleEventLoop,
    };
    scale::presence::PresenceConfig presenceConfig = {
        .detectorGPIO = GPIO_PRESENCE,
        .eventLoop = scaleEventLoop,
    };
    scale::presence::PresenceDetector presenceDetector(presenceConfig);
    scale::ota::OTA ota(otaConfig);
    ota.start();

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}