#pragma once

#include "esp_event.h"
#include "i2c-lcd1602.h"

#include "scale/widgets/events.h"

namespace scale::lcd {
class BaseWidget {
   public:
    BaseWidget(
        i2c_lcd1602_info_t *lcdInfo,
        esp_event_loop_handle_t eventLoop,
        esp_event_loop_handle_t lcdEventLoop)
        : _lcdInfo(lcdInfo), _eventLoop(eventLoop), _lcdEventLoop(lcdEventLoop) {}

   protected:
    virtual void render() = 0;

    template <typename E>
    void postEvent(E &event_data) {
        esp_event_post_to(
            _lcdEventLoop,
            events::LCD_EVENT,
            E::event_id,
            &event_data,
            sizeof(event_data),
            0
        );
    }

    esp_event_loop_handle_t eventLoop() {
        return _eventLoop;
    }
    esp_event_loop_handle_t lcdEventLoop() {
        return _lcdEventLoop;
    }
    i2c_lcd1602_info_t *lcdInfo() {
        return _lcdInfo;
    }

   private:
    i2c_lcd1602_info_t *_lcdInfo;
    esp_event_loop_handle_t _eventLoop;
    esp_event_loop_handle_t _lcdEventLoop;
};
}  // namespace scale::lcd