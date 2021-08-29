#pragma once

#include "scale_button.h"
#include "stabilized_scale.h"
#include "scale/tare.h"
#include "color_report.h"
#include "mqtt_report.h"
#include "maintenance.h"
#include "scale_lcd.h"

namespace scale::controller {
    struct ScaleControllerArgs {
        peri::button::PushButton &buttonTare;
        peri::button::PushButton &buttonMaintenance;
        maintenance::Maintenance &maintenance;
        color::ColorReport &colorReport;
        stabilized::StabilizedScale &stabilizedScale;
        tare::Tare &tare;
        tare::TareConfigBuilder &tareConfigBuilder;
        mqtt::MQTTReport &mqttReport;
        lcd::LCD &lcd;
    };

    class ScaleController {
    public:
        ScaleController(ScaleControllerArgs &args): 
                _buttonTare(args.buttonTare),
                _buttonMaintenance(args.buttonMaintenance),
                _maintenance(args.maintenance),
                _colorReport(args.colorReport),
                _stabilizedScale(args.stabilizedScale),
                _tare(args.tare),
                _tareConfigBuilder(args.tareConfigBuilder),
                _mqttReport(args.mqttReport),
                _lcd(args.lcd) {

        }

        void start();
    private:
        void processScaleEvent(const stabilized::ScaleEvent &scaleEvent);
        void onTareButtonPressed();
        void onMaintenanceButtonPressed();

        bool _taring = false;

        peri::button::PushButton &_buttonTare;
        peri::button::PushButton &_buttonMaintenance;
        maintenance::Maintenance &_maintenance;
        color::ColorReport &_colorReport;
        stabilized::StabilizedScale &_stabilizedScale;
        tare::Tare &_tare;
        tare::TareConfigBuilder &_tareConfigBuilder;
        mqtt::MQTTReport &_mqttReport;
        lcd::LCD &_lcd;
    };
}