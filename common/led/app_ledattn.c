#include "driver/gpio.h"
#include "app_ledattn.h"

void appled_init(void)
{
    gpio_reset_pin(ATTN_LED_PIN);
    gpio_set_direction(ATTN_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(ATTN_LED_PIN, 0);
}

void appled_set(bool on)
{
    gpio_set_level(ATTN_LED_PIN, on ? 1 : 0);
}