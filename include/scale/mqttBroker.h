

#include "scale/events.h"

namespace scale::mqtt {
struct MQTTBrokerConfig {
    esp_event_loop_handle_t eventLoop;
    std::string topicUpdate;
};
class MQTTBroker {
   public:
    MQTTBroker(const MQTTBrokerConfig &config);

   private:
    MQTTBrokerConfig _config;
};
}  // namespace scale::mqtt