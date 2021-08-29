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
        subscribeToEvents();
    }

    void LCD::start() {
        xTaskCreate(
            [](void *arg) {
                LCD &_this = *((LCD*)arg);
                _this.init();
                std::unique_ptr<BaseWidget> wifiWidget(new WifiWidget(_this._lcdInfo, _this._config.eventLoop));
                std::unique_ptr<BaseWidget> mqttWidget(new MQTTWidget(_this._lcdInfo, _this._config.eventLoop));
                std::unique_ptr<BaseWidget> weightWidget(new WeightWidget(_this._lcdInfo, _this._config.eventLoop));
                std::unique_ptr<BaseWidget> maintenanceWidget(new MaintenanceWidget(_this._lcdInfo, _this._config.eventLoop));

                _this._widgets.push_back(std::move(wifiWidget));
                _this._widgets.push_back(std::move(mqttWidget));
                _this._widgets.push_back(std::move(weightWidget));
                _this._widgets.push_back(std::move(maintenanceWidget));
                xQueueSend(_this._readyQueue, nullptr, portMAX_DELAY);
                while (true) {
                    _this.taskLoop();
                }
            }, "LCD", 8192, this, 10, nullptr
        );
    }
    
    void LCD::waitUntilReady() {
        xQueueReceive(_readyQueue, nullptr, portMAX_DELAY);
    }

    void LCD::taskLoop() {
        LCDEvent evt;
        if (!xQueueReceive(_eventQueue, &evt, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Timeout waiting for event");
        }
        render();
    }

    void LCD::render() {
        for (auto &widget : _widgets) {
            widget->render();
        }
    }

    void LCD::requestRedraw() {
        LCDEvent event;
        xQueueSend(_eventQueue, &event, portMAX_DELAY);
    }

    void LCD::subscribeToEvents() {
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