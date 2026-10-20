
#include "app_sw.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "switch_hw"

static bool led_state = false;

void init_hw(void)
{
    gpio_reset_pin(SWITCH_LED_PIN);
    gpio_set_direction(SWITCH_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(SWITCH_LED_PIN, 0);

    led_state = false;

    ESP_LOGI(TAG, "Switch LED initialized");
}

void switch_set(bool on)
{
    led_state = on;
    gpio_set_level(SWITCH_LED_PIN, on ? 1 : 0);

    ESP_LOGI(TAG, "LED is %s", on ? "ON" : "OFF");
}

bool switch_get(void)
{
    return led_state;
}
