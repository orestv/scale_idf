#pragma once

#include "scale_button.h"
#include "stabilized_scale.h"
#include "tare_controller.h"
#include "color_report.h"
#include "mqtt.h"

namespace scale::controller {
    struct ScaleControllerArgs {
        peri::button::PushButton &buttonTare;
        color::ColorReport &colorReport;
        stabilized::StabilizedScale &stabilizedScale;
        tare::Tare &tare;
        tare::TareConfigBuilder &tareConfigBuilder;
        mqtt::MQTTClient &mqttClient;
    };

    class MQTTReportDebouncer {
    public:
        MQTTReportDebouncer(uint periodMS);
        bool shouldPublish(const mqtt::OutgoingMQTTMessage &msg) const;
        void published(const mqtt::OutgoingMQTTMessage &msg);
        void clear();
    private:
        uint now() const;

        uint _periodMS;

        mqtt::OutgoingMQTTMessage _storedMessage;
        bool _published;
        int _lastPublishTime;
    };

    class ScaleController {
    public:
        ScaleController(ScaleControllerArgs &args): 
                _buttonTare(args.buttonTare),
                _colorReport(args.colorReport),
                _stabilizedScale(args.stabilizedScale),
                _tare(args.tare),
                _tareConfigBuilder(args.tareConfigBuilder),
                _mqttClient(args.mqttClient),
                _reportDebouncer(1000U) {

        }

        void start();
    private:
        void processScaleEvent(const stabilized::ScaleEvent &scaleEvent);
        void onTareButtonPressed();

        bool _taring = false;

        peri::button::PushButton &_buttonTare;
        color::ColorReport &_colorReport;
        stabilized::StabilizedScale &_stabilizedScale;
        tare::Tare &_tare;
        tare::TareConfigBuilder &_tareConfigBuilder;
        mqtt::MQTTClient &_mqttClient;

        MQTTReportDebouncer _reportDebouncer;
    };
}