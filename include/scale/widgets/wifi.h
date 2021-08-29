#pragma once

#include "scale/widgets/base.h"

#include "i2c-lcd1602.h"
#include "scale/events.h"

namespace scale::lcd {
class WifiWidget: public BaseWidget {
   public:
    WifiWidget(i2c_lcd1602_info_t *lcdInfo, esp_event_loop_handle_t eventLoop);
    virtual void render();
   private:
    i2c_lcd1602_info_t *_lcdInfo;
    esp_event_loop_handle_t _eventLoop;

    bool _wifiConnected = false;
};
}  // namespace scale::lcd