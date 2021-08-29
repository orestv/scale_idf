#include "scale/lcd.h"

#include "sstream"
#include <iomanip>

namespace scale::lcd {
    const char *TAG = "LCD";

    LCD::LCD(const LCDConfig &config): _config(config), _lcdInfo(nullptr) {
        _eventQueue = xQueueCreate(10, sizeof(LCDEvent));
        _state = {
            .wifiConnected = false,
            .mqttConnected = false,
            .grams = 0,
        };
        subscribeToEvents();
    }

    void LCD::start() {
        xTaskCreate(
            [](void *arg) {
                LCD &_this = *((LCD*)arg);
                _this.init();
                while (true) {
                    _this.taskLoop();
                }
            }, "LCD", 8192, this, 10, nullptr
        );
    }

    void LCD::taskLoop() {
        LCDEvent evt;
        if (!xQueueReceive(_eventQueue, &evt, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Timeout waiting for event");
        }
        render();
    }

    void LCD::render() {
        renderWifi();
        renderMQTT();
        renderWeight();
        renderMaintenance();
    }
    void LCD::renderWifi() {
        i2c_lcd1602_move_cursor(_lcdInfo, 18, 0);
        char mqttStateChar = _state.wifiConnected ? '+' : '-';
        i2c_lcd1602_write_char(_lcdInfo, 'W');
        i2c_lcd1602_move_cursor(_lcdInfo, 19, 0);
        i2c_lcd1602_write_char(_lcdInfo, mqttStateChar);
    }
    void LCD::renderMQTT() {
        i2c_lcd1602_move_cursor(_lcdInfo, 18, 1);
        char mqttStateChar = _state.mqttConnected ? '+' : '-';
        i2c_lcd1602_write_char(_lcdInfo, 'M');
        i2c_lcd1602_move_cursor(_lcdInfo, 19, 1);
        i2c_lcd1602_write_char(_lcdInfo, mqttStateChar);
    }
    void LCD::renderWeight() {
        std::stringstream oss;
        oss << "Weight: " << std::fixed << std::setprecision(2) << _state.grams;

        i2c_lcd1602_move_cursor(_lcdInfo, 0, 3);
        i2c_lcd1602_write_string(_lcdInfo, oss.str().c_str());
    }

    void LCD::renderMaintenance() {
        const char *sMaintenance = _state.maintenance ? "MAINTENANCE" : "           ";
        i2c_lcd1602_move_cursor(_lcdInfo, 0, 0);
        i2c_lcd1602_write_string(_lcdInfo, sMaintenance);
    }

    void LCD::setWifiState(bool wifiConnected) {
        _state.wifiConnected = wifiConnected;
        requestRedraw();
    }

    void LCD::setMQTTState(bool mqttConnected) {
        _state.mqttConnected = mqttConnected;
        requestRedraw();
    }

    void LCD::setWeight(float grams) {
        _state.grams = grams;
        requestRedraw();
    }

    void LCD::setMaintenance(bool maintenance) {
        _state.maintenance = maintenance;
        requestRedraw();
    }

    void LCD::requestRedraw() {
        LCDEvent event;
        xQueueSend(_eventQueue, &event, portMAX_DELAY);
    }

    void LCD::onWeightChanged(const events::EventStabilizedTaredWeightChanged &evt) {
        setWeight(evt.grams);
    }

    void LCD::subscribeToEvents() {
        esp_event_handler_register_with(
            _config.eventLoop,
            events::SCALE_EVENT,
            events::EVENT_STABILIZED_TARED_WEIGHT_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                LCD &_this = *(LCD*)arg;
                auto &evt = *(events::EventStabilizedTaredWeightChanged*)event_data;
                _this.onWeightChanged(evt);
            }, 
            this
        );
        esp_event_handler_register_with(
            _config.eventLoop,
            events::SCALE_EVENT,
            events::EVENT_WIFI_CONNECTION_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                LCD &_this = *(LCD *)arg;
                auto &evt = *(events::EventWifiConnectionChanged *)event_data;
                _this.setWifiState(evt.connected);
            },
            this);
        esp_event_handler_register_with(
            _config.eventLoop,
            events::SCALE_EVENT,
            events::EVENT_MQTT_CONNECTION_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                LCD &_this = *(LCD *)arg;
                auto &evt = *(events::EventMQTTConnectionChanged *)event_data;
                _this.setMQTTState(evt.connected);
            },
            this);
        esp_event_handler_register_with(
            _config.eventLoop,
            events::SCALE_EVENT,
            events::EVENT_MAINTENANCE_MODE_CHANGED,
            [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
                LCD &_this = *(LCD *)arg;
                auto &evt = *(events::EventMaintenanceModeChanged *)event_data;
                _this.setMaintenance(evt.isMaintenanceModeOn);
            },
            this);
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
    }
}