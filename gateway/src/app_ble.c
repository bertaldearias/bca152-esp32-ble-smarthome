#include "esp_log.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_health_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "app_ble.h"

#define TAG "gateway_ble"
#define CID_ESP 0x02E5

#ifndef ROLE_NODE
#define ROLE_NODE 0
#endif

static uint8_t dev_uuid[16] = { 0xdd, 0xdd };

static esp_ble_mesh_cfg_srv_t config_server = {
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
    .default_ttl = 7,
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(health_pub_0, 2 + 3, ROLE_NODE);
static esp_ble_mesh_health_srv_t health_server;

static esp_ble_mesh_client_t onoff_client;
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_cli_pub, 2 + 3, ROLE_NODE);

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_CLI(&onoff_cli_pub, &onoff_client),
    ESP_BLE_MESH_MODEL_HEALTH_SRV(&health_server, &health_pub_0),
};

#define ONOFF_CLIENT_INDEX 1

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .element_count = sizeof(elements) / sizeof(elements[0]),
    .elements = elements,
};

static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
};

static void generic_client_cb(esp_ble_mesh_generic_client_cb_event_t event,
                              esp_ble_mesh_generic_client_cb_param_t *param)
{
    ESP_LOGI(TAG, "generic client event %d, error_code %d",
             event, param->error_code);
}

void init_ble(appble_attn_cbs_t cbs)
{
    esp_err_t err;

    appble_set_attn_cbs(cbs);

    err = appble_bt_init();
    if (err) {
        ESP_LOGE(TAG, "bluetooth init failed: %s", esp_err_to_name(err));
        return;
    }

    appble_get_dev_uuid(dev_uuid);

    esp_ble_mesh_register_prov_callback(appble_provisioning_handler);
    esp_ble_mesh_register_config_server_callback(appble_config_handler);
    esp_ble_mesh_register_health_server_callback(appble_health_evt_handler);
    esp_ble_mesh_register_generic_client_callback(generic_client_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err) {
        ESP_LOGE(TAG, "mesh init failed: %s", esp_err_to_name(err));
        return;
    }

    err = esp_ble_mesh_node_prov_enable(
        ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err) {
        ESP_LOGE(TAG, "prov enable failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "BLE mesh node initialized");
}

void appble_set_switch(bool on)
{
    static uint8_t tid = 0;
    esp_ble_mesh_model_t *model = &root_models[ONOFF_CLIENT_INDEX];

    if (model->pub == NULL ||
        model->pub->publish_addr == ESP_BLE_MESH_ADDR_UNASSIGNED) {
        ESP_LOGW(TAG, "no publish address set for the OnOff client yet");
        return;
    }

    uint8_t data[2] = { on ? 1 : 0, tid++ };

    esp_err_t err = esp_ble_mesh_model_publish(
        model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK,
        sizeof(data), data, ROLE_NODE);
    if (err) {
        ESP_LOGE(TAG, "publish failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "sent OnOff %s", on ? "ON" : "OFF");
    }
}