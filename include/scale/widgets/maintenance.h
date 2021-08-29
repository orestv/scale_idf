#pragma once

#include "base.h"

namespace scale::lcd {
class MaintenanceWidget : public BaseWidget {
   public:
    MaintenanceWidget(i2c_lcd1602_info_t *lcdInfo,
                 esp_event_loop_handle_t eventLoop);
    void render();

   private:
    bool _maintenanceOn;
};
}  // namespace scale::lcd