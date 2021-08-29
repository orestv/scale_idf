#pragma once

#pragma once

#include "i2c-lcd1602.h"
#include "scale/events.h"
#include "scale/widgets/base.h"

#include "freertos/queue.h"

namespace scale::lcd {
class WeightReportWidget : public BaseWidget {
   public:
    WeightReportWidget(i2c_lcd1602_info_t *lcdInfo, esp_event_loop_handle_t eventLoop);
    virtual void render();

   private:
    void blip();
    void renderBlip();

    xQueueHandle _blipQueue;    
};
}  // namespace scale::lcd