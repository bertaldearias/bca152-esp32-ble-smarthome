#include "esp_log.h"
#include "app_ip.h"

#define TAG "gateway"

static void wifi_connected(void)
{
    ESP_LOGI(TAG, "wifi connected");
}

static void wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main(void)
{
    ESP_LOGI(TAG, "gateway starting");

    connect_wifi_params_t p = {
        .on_connected = wifi_connected,
        .on_failed = wifi_failed,
    };
    appip_connect_wifi(p);
}