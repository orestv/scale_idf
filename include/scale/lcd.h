#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "rom/uart.h"

#include "scale/events.h"

#include "widgets/base.h"

#include <memory>
#include <vector>

#include "smbus.h"
#include "i2c-lcd1602.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TX_BUF_LEN 0 // disabled
#define I2C_MASTER_RX_BUF_LEN 0 // disabled
#define I2C_MASTER_FREQ_HZ 100000


namespace scale::lcd {

    struct LCDConfig {
        gpio_num_t gpioSDA;
        gpio_num_t gpioSCL;
        esp_event_loop_handle_t eventLoop;
    };

    struct LCDEvent {

    };

    struct LCDState {
    };

    class LCD {
    public:
        LCD(const LCDConfig &config);

        void start();
        void waitUntilReady();
    private:

        void init();

        xQueueHandle _eventQueue;
        xQueueHandle _readyQueue;

        std::vector<std::unique_ptr<BaseWidget>> _widgets;

        esp_event_loop_handle_t _eventLoop;

        LCDState _state;

        LCDConfig _config;
        i2c_lcd1602_info_t *_lcdInfo;
    };
}