#include <iomanip>

#include "scale/lcd.h"
#include "sstream"

#include "scale/widgets/widgets.h"

namespace scale::lcd {
    const char *TAG = "LCD";

    LCD::LCD(const LCDConfig &config): _config(config), _lcdInfo(nullptr) {
        _eventQueue = xQueueCreate(10, sizeof(LCDEvent));
        _readyQueue = xQueueCreate(1, 0);
        _state = {
        };        
    }

    void LCD::start() {
        esp_event_loop_args_t eventLoopArgs = {
            .queue_size = 5,
            .task_name = "LCDEvents",
            .task_priority = 50,
            .task_stack_size = 8192,
        };
        esp_event_loop_create(&eventLoopArgs, &_eventLoop);

        xTaskCreate(
            [](void *arg) {
                LCD &_this = *((LCD *)arg);

                const int LCD_TIMEOUT_MS = 5000;
                const int LCD_TIMEOUT_TICKS = pdMS_TO_TICKS(LCD_TIMEOUT_MS);
                while (true) {
                    auto xNotifyCount = ulTaskNotifyTake(true, LCD_TIMEOUT_TICKS);
                    if (xNotifyCount == 0) {
                        ESP_LOGI(TAG, "LCD Backlight Timeout");

                        events::EventBacklightStateChanged evt;
                        evt.isBacklightOn = false;
                        // esp_event_post_to(
                        //     _this._eventLoop,
                        //     events::LCD_EVENT,
                        //     events::EVENT_BACKLIGHT_STATE_CHANGED,
                        //     &evt, sizeof(&evt), 0);
                    } else {
                        ESP_LOGI(TAG, "Movement detected, backlight timeout delayed");
                    }
                }
            },
            "BacklightTimeout", 2048, this, 2, &_taskBacklightTimeout);

        xTaskCreate(
            [](void *arg) {
                LCD &_this = *((LCD*)arg);
                _this.init();

                WifiWidget wifiWidget(_this._lcdInfo, _this._config.eventLoop, _this._eventLoop);
                MQTTWidget mqttWidget(_this._lcdInfo, _this._config.eventLoop, _this._eventLoop);
                WeightWidget weightWidget(_this._lcdInfo, _this._config.eventLoop, _this._eventLoop);
                MaintenanceWidget maintenanceWidget(_this._lcdInfo, _this._config.eventLoop, _this._eventLoop);
                WeightReportWidget weightReportWidget(_this._lcdInfo, _this._config.eventLoop, _this._eventLoop);
                UpdateWidget updateWidget(_this._lcdInfo, _this._config.eventLoop, _this._eventLoop);

                xQueueSend(_this._readyQueue, nullptr, portMAX_DELAY);
                while (true) {
                    vTaskDelay(portMAX_DELAY);
                }
            }, "LCD", 8192, this, 10, nullptr
        );
        esp_event_handler_register_with(
            _config.eventLoop, scale::events::SCALE_EVENT,
            scale::events::EVENT_MOVEMENT_DETECTED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                ESP_LOGI(TAG, "Movement detected");
                auto &_this = *(LCD*)arg;
                events::EventBacklightStateChanged evt;
                evt.isBacklightOn = true;
                esp_event_post_to(
                    _this._eventLoop,
                    events::LCD_EVENT,
                    events::EVENT_BACKLIGHT_STATE_CHANGED,
                    &evt, sizeof(&evt), 0
                );
                xTaskNotifyGive(_this._taskBacklightTimeout);
            }, this
        );

        esp_event_handler_register_with(
            _eventLoop, events::LCD_EVENT, events::EVENT_BACKLIGHT_STATE_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                auto &_this = *(LCD *)arg;
                auto &evt = *(events::EventBacklightStateChanged*)event_data;
                i2c_lcd1602_set_backlight(_this._lcdInfo, evt.isBacklightOn);
            },
            this);
    }
    
    void LCD::waitUntilReady() {
        xQueueReceive(_readyQueue, nullptr, portMAX_DELAY);
    }

    void LCD::init() {
        int i2c_master_port = I2C_NUM_0;
        i2c_port_t i2c_num = I2C_MASTER_NUM;
        i2c_config_t conf;
        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = _config.gpioSDA;
        conf.sda_pullup_en = GPIO_PULLUP_DISABLE; // GY-2561 provides 10kΩ pullups
        conf.scl_io_num = _config.gpioSCL;
        conf.scl_pullup_en = GPIO_PULLUP_DISABLE; // GY-2561 provides 10kΩ pullups
        conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
        conf.clk_flags = 0;
        i2c_param_config(i2c_master_port, &conf);
        i2c_driver_install(i2c_master_port, conf.mode,
                           0,
                           0, 0);

        uint8_t address = 0x27;

        // Set up the SMBus
        smbus_info_t *smbus_info = smbus_malloc();
        ESP_ERROR_CHECK(smbus_init(smbus_info, i2c_num, address));
        ESP_ERROR_CHECK(smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS));

        // Set up the LCD1602 device with backlight off
        _lcdInfo = i2c_lcd1602_malloc();
        ESP_ERROR_CHECK(i2c_lcd1602_init(_lcdInfo, smbus_info, true,
                                         4, 20, 20));

        ESP_ERROR_CHECK(i2c_lcd1602_reset(_lcdInfo));
        i2c_lcd1602_clear(_lcdInfo);
    }
}