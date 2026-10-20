#include <string.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "app_blecommon.h"

#define TAG "blecommon"
#define BD_ADDR_BYTES 6

static appble_attn_cbs_t attn_cbs;

void appble_set_attn_cbs(appble_attn_cbs_t cbs)
{
    attn_cbs = cbs;
}

void appble_get_dev_uuid(uint8_t *dev_uuid)
{
    memcpy(dev_uuid + 2, esp_bt_dev_get_address(), BD_ADDR_BYTES);
}

esp_err_t appble_bt_init(void)
{
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "controller init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "controller enable failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "bluedroid init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "bluedroid enable failed: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

void appble_provisioning_handler(esp_ble_mesh_prov_cb_event_t event,
                                 esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(TAG, "mesh registered, err_code %d",
                 param->prov_register_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(TAG, "provisioning enabled, err_code %d",
                 param->node_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
        ESP_LOGI(TAG, "provisioning link open, bearer %s",
                 param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV
                     ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(TAG, "provisioning link closed, bearer %s",
                 param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV
                     ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(TAG, "provisioned, net_idx 0x%04x, addr 0x%04x",
                 param->node_prov_complete.net_idx,
                 param->node_prov_complete.addr);
        break;
    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(TAG, "node reset");
        break;
    default:
        break;
    }
}

void appble_config_handler(esp_ble_mesh_cfg_server_cb_event_t event,
                           esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event != ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        return;
    }

    switch (param->ctx.recv_op) {
    case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
        ESP_LOGI(TAG, "app key added, net_idx 0x%04x, app_idx 0x%04x",
                 param->value.state_change.appkey_add.net_idx,
                 param->value.state_change.appkey_add.app_idx);
        break;
    case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
        ESP_LOGI(TAG, "model app bind, elem 0x%04x, app_idx 0x%04x, model 0x%04x",
                 param->value.state_change.mod_app_bind.element_addr,
                 param->value.state_change.mod_app_bind.app_idx,
                 param->value.state_change.mod_app_bind.model_id);
        break;
    default:
        break;
    }
}

void appble_health_evt_handler(esp_ble_mesh_health_server_cb_event_t event,
                               esp_ble_mesh_health_server_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_HEALTH_SERVER_ATTENTION_ON_EVT:
        ESP_LOGI(TAG, "attention on");
        if (attn_cbs.attn_on) {
            attn_cbs.attn_on();
        }
        break;
    case ESP_BLE_MESH_HEALTH_SERVER_ATTENTION_OFF_EVT:
        ESP_LOGI(TAG, "attention off");
        if (attn_cbs.attn_off) {
            attn_cbs.attn_off();
        }
        break;
    default:
        break;
    }
}