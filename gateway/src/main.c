#include "esp_log.h"
#include "app_ip.h"
#include "app_web.h"

#define TAG "gateway"

static void on_switch(bool on)
{
    ESP_LOGI(TAG, "switch request: %s", on ? "ON" : "OFF");
}

static void wifi_connected(void)
{
    ESP_LOGI(TAG, "wifi connected");
    appweb_start_server();
}

static void wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main(void)
{
    ESP_LOGI(TAG, "gateway starting");

    appweb_init(on_switch);

    connect_wifi_params_t p = {
        .on_connected = wifi_connected,
        .on_failed = wifi_failed,
    };
    appip_connect_wifi(p);
}