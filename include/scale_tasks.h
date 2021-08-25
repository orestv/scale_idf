#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "esp_log.h"

#include "scale_button.h"
#include "adapted_scale.h"

namespace scale::tasks {

    struct TaskArgTareButton {
        scale::weight::AdaptedScale &scale;
        scale::peri::button::PushButton &button;
    };

    void taskTareButton(void *arg) {
        TaskArgTareButton &taskArg = *(TaskArgTareButton*)arg;

        ESP_LOGI("TaskTareButton", "Starting task");

        while (true) {
            scale::peri::button::ButtonEvent evt;
            if (xQueueReceive(taskArg.button.queue(), &evt, portMAX_DELAY)) {
                ESP_LOGI("TaskTareButton", "Received button event");
            }
        }
    }

    void startTaskTareButton(TaskArgTareButton &arg) {
        xTaskCreate(taskTareButton, "TareButton", 2048, &arg, 10, nullptr);
    }
}