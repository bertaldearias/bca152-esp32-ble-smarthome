#include <string.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "app_web.h"

#define TAG "app_web"

static set_switch_f set_switch_cb = NULL;

static const char *HTML_FORM =
    "<html><form action=\"/\" method=\"post\">"
    "<label for=\"switch_state\">Set switch:</label>"
    "<select id=\"switch_state\" name=\"switch_state\">"
    "<option value=\"ON\">ON</option>"
    "<option value=\"OFF\">OFF</option>"
    "</select>"
    "<input type=\"submit\" value=\"Submit\">"
    "</form></html>";

static esp_err_t get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, HTML_FORM, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t post_handler(httpd_req_t *req)
{
    char buf[64];
    int total = req->content_len;

    if (total <= 0 || total >= (int)sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad request");
        return ESP_FAIL;
    }

    int received = 0;
    while (received < total) {
        int r = httpd_req_recv(req, buf + received, total - received);
        if (r == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;
        }
        if (r <= 0) {
            return ESP_FAIL;
        }
        received += r;
    }
    buf[received] = '\0';

    ESP_LOGI(TAG, "request body: %s", buf);

    if (strstr(buf, "switch_state=ON") != NULL) {
        ESP_LOGI(TAG, "user requested switch ON");
        if (set_switch_cb) {
            set_switch_cb(true);
        }
    } else if (strstr(buf, "switch_state=OFF") != NULL) {
        ESP_LOGI(TAG, "user requested switch OFF");
        if (set_switch_cb) {
            set_switch_cb(false);
        }
    }

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, HTML_FORM, HTTPD_RESP_USE_STRLEN);
}

static const httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t uri_post = {
    .uri = "/",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL,
};

void appweb_init(set_switch_f cb)
{
    set_switch_cb = cb;
}

void appweb_start_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
        ESP_LOGI(TAG, "web server started");
    } else {
        ESP_LOGE(TAG, "failed to start web server");
    }
}