#include "scale/mqtt_logger.h"
#include "esp_log.h"

namespace scale::logger {

static MQTTLogger *logger = nullptr;

int MQTTLogger::mqtt_vprintf(const char *format, va_list arg_ptr) {
    char buf[256];
    vsprintf(buf, format, arg_ptr);
    events::EventLogMessage evt({
        .logMessage=std::string(buf),
    });
    esp_event_post_to(
        logger->_config.eventLoop,
        events::SCALE_EVENT,
        events::EVENT_LOG_MESSAGE,
        &evt,
        sizeof(evt),
        portMAX_DELAY
    );
    return vprintf(format, arg_ptr);
}
MQTTLogger::MQTTLogger(const MQTTLoggerConfig &config) : _config(config) {
    logger = this;
    // esp_log_set_vprintf(&MQTTLogger::mqtt_vprintf);
}
}  // namespace scale::logger