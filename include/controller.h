#pragma once

#include "scale_button.h"
#include "stabilized_scale.h"
#include "tare_controller.h"
#include "color_report.h"
#include "mqtt_report.h"

namespace scale::controller {
    struct ScaleControllerArgs {
        peri::button::PushButton &buttonTare;
        color::ColorReport &colorReport;
        stabilized::StabilizedScale &stabilizedScale;
        tare::Tare &tare;
        tare::TareConfigBuilder &tareConfigBuilder;
        mqtt::MQTTReport &mqttReport;
    };

    class ScaleController {
    public:
        ScaleController(ScaleControllerArgs &args): 
                _buttonTare(args.buttonTare),
                _colorReport(args.colorReport),
                _stabilizedScale(args.stabilizedScale),
                _tare(args.tare),
                _tareConfigBuilder(args.tareConfigBuilder),
                _mqttReport(args.mqttReport) {

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
        mqtt::MQTTReport &_mqttReport;
    };
}