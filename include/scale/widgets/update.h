#pragma once

#include "base.h"

namespace scale::lcd {
    class UpdateWidget : public BaseWidget {
       public:
        UpdateWidget(i2c_lcd1602_info_t *lcdInfo, esp_event_loop_handle_t eventLoop, esp_event_loop_handle_t lcdEventLoop);
        void render();

       private:
        lcd::events::EventUpdateStateChanged _lastEvent;
    };
}