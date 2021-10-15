#include "scale/events.h"

#include <string>

namespace scale::logger {
struct MQTTLoggerConfig {
    esp_event_loop_handle_t eventLoop;
    std::string logTopic;
};
class MQTTLogger {
   public:
    MQTTLogger(const MQTTLoggerConfig &config);

   private:
    static int mqtt_vprintf(const char *format, va_list arg_ptr);

    MQTTLoggerConfig _config;
};
}  // namespace scale::logger