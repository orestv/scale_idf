#pragma once

#include "i2c-lcd1602.h"
#include "scale/events.h"
#include "scale/widgets/base.h"

namespace scale::lcd {
class WifiWidget : public BaseWidget {
   public:
    WifiWidget(i2c_lcd1602_info_t *lcdInfo, esp_event_loop_handle_t eventLoop, esp_event_loop_handle_t lcdEventLoop);
    virtual void render();

    bool _wifiConnected = false;
};
}  // namespace scale::lcd