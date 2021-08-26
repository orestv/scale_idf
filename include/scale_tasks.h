#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "esp_log.h"

#include "tare_controller.h"
#include "scale_button.h"
#include "state_machine.h"

namespace scale::tasks {

    struct TaskArgTareButton {
        scale::state::StateMachine &stateMachine;
        scale::peri::button::PushButton &button;
        scale::tare::TareConfigBuilder &tareConfigBuilder;
    };

    struct TaskArgReportWeight {
        scale::adapted::AdaptedScale &adaptedScale;
        scale::state::StateMachine &stateMachine;
        scale::tare::Tare &tare;
        scale::tare::TareConfigBuilder &tareConfigBuilder;
    };

    void taskTareButton(void *arg) {
        TaskArgTareButton &taskArg = *(TaskArgTareButton*)arg;

        ESP_LOGI("TaskTareButton", "Starting task");

        while (true) {
            scale::peri::button::ButtonEvent evt;
            if (xQueueReceive(taskArg.button.queue(), &evt, portMAX_DELAY)) {
                ESP_LOGI("TaskTareButton", "Received button event");
                taskArg.stateMachine.setState(scale::state::SCALE_STATE_TARE);
                taskArg.tareConfigBuilder.reset();
            }
        }
    }

    void taskReportWeight(void *arg) {
        TaskArgReportWeight &taskArg = *(TaskArgReportWeight*)arg;
        const char *TAG = "ReportWeight";
        while (true) {
            scale::adapted::ScaleEvent event;
            if (xQueueReceive(taskArg.adaptedScale.queue(), &event, portMAX_DELAY)) {
                // ESP_LOGI(TAG, "Received weight: %fg", event.grams);                

                switch (taskArg.stateMachine.state()) {
                    case scale::state::SCALE_STATE_NORMAL: {
                        float taredWeight = taskArg.tare.tare(event.grams);
                        ESP_LOGI(TAG, "                   %fg", taredWeight);
                        break;
                    }
                    case scale::state::SCALE_STATE_TARE: {
                        ESP_LOGI(TAG, "Pushing value into tare");
                        taskArg.tareConfigBuilder.push(event.grams);
                        if (taskArg.tareConfigBuilder.isTareReady()) {
                            ESP_LOGI(TAG, "Tare ready");
                            taskArg.tare.configure(taskArg.tareConfigBuilder.config());
                            taskArg.tareConfigBuilder.reset();
                            taskArg.stateMachine.setState(scale::state::SCALE_STATE_NORMAL);
                        }
                    }
                }
            }
        }
    }

    void startTaskTareButton(TaskArgTareButton &arg) {
        xTaskCreate(taskTareButton, "TareButton", 2048, &arg, 10, nullptr);
    }

    void startTaskReportWeight(TaskArgReportWeight &arg) {
        xTaskCreate(taskReportWeight, "ReportWeight", 2048, &arg, 10, nullptr);
    }
}