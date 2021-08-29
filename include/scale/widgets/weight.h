#pragma once

#include "base.h"

namespace scale::lcd {
class WeightWidget : public BaseWidget {
   public:
    WeightWidget(i2c_lcd1602_info_t *lcdInfo,
                 esp_event_loop_handle_t eventLoop);
    void render();

   private:
    float _grams;
};
}  // namespace scale::lcd