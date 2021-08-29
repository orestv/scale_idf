#pragma once

#include "esp_event.h"
#include "i2c-lcd1602.h"

namespace scale::lcd {
class BaseWidget {
   public:
    BaseWidget(
        i2c_lcd1602_info_t *lcdInfo,
        esp_event_loop_handle_t eventLoop)
        : _lcdInfo(lcdInfo), _eventLoop(eventLoop) {}

    virtual void render() = 0;

   protected:
    esp_event_loop_handle_t eventLoop() {
        return _eventLoop;
    }
    i2c_lcd1602_info_t *lcdInfo() {
        return _lcdInfo;
    }

   private:
    i2c_lcd1602_info_t *_lcdInfo;
    esp_event_loop_handle_t _eventLoop;
};
}  // namespace scale::lcd