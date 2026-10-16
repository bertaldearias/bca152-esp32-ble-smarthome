#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "app_sensor.h"

#define TAG "sensor"

static adc_oneshot_unit_handle_t adc_handle;
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
        int raw = 0;
        adc_oneshot_read(adc_handle, LIGHT_ADC_CHANNEL, &raw);
        ESP_LOGI(TAG, "light level: %d", raw);

        bool low = raw < LIGHT_THRESHOLD;
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

    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, LIGHT_ADC_CHANNEL, &chan_cfg));

    xTaskCreate(sensor_task, "sensor_task", 3072, NULL, 5, NULL);
}