#pragma once

#include <freertos/FreeRTOS.h>
#include <hx711.h>
#include <freertos/task.h>


#define PD_SCK_GPIO GPIO_NUM_18
#define DOUT_GPIO   GPIO_NUM_19

void test(void *pvParameters)
{
    hx711_t dev = {
        .dout = DOUT_GPIO,
        .pd_sck = PD_SCK_GPIO,
        .gain = HX711_GAIN_A_128
    };

    // initialize device
    while (1)
    {
        esp_err_t r = hx711_init(&dev);
        if (r == ESP_OK)
            break;
        printf("Could not initialize HX711: %d (%s)\n", r, esp_err_to_name(r));
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // read from device
    while (1)
    {
        esp_err_t r = hx711_wait(&dev, 500);
        if (r != ESP_OK)
        {
            printf("Device not found: %d (%s)\n", r, esp_err_to_name(r));
            continue;
        }

        int32_t data;
        r = hx711_read_data(&dev, &data);
        if (r != ESP_OK)
        {
            printf("Could not read data: %d (%s)\n", r, esp_err_to_name(r));
            continue;
        }

        printf("Raw data: %d\n", data);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}