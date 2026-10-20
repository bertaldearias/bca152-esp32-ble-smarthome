#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "app_sensor.h"

#define TAG "sensor"

static light_changed_f changed_cb;
static bool light_low = false;

bool is_light_low(void)
{
    return light_low;
}

static void sensor_task(void *arg)
{
    vTaskDelay(pdMS_TO_TICKS(5000));   /* wait for BLE mesh to start */
    bool first = true;
    while (1) {
        int level = gpio_get_level(LIGHT_DO_PIN);
        bool low = (level == DARK_LEVEL);
        ESP_LOGI(TAG, "D0 level: %d (%s)", level, low ? "dark" : "light");

        if (first || low != light_low) {
            light_low = low;
            first = false;
            if (changed_cb) {
                changed_cb(light_low);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void init_hw(light_changed_f cb)
{
    changed_cb = cb;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LIGHT_DO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    xTaskCreate(sensor_task, "sensor_task", 3072, NULL, 5, NULL);
}