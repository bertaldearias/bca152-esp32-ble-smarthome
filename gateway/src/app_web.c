
#include <string.h>
#include <stdio.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "app_web.h"

#define TAG "app_web"

static set_switch_f set_switch_cb = NULL;
static bool switch_is_on = false;

static const char *HTML_PAGE =
    "<!DOCTYPE html>"
    "<html lang='en'>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>Smart Switch</title>"
    "<style>"
    "*{box-sizing:border-box;}"
    "body{margin:0;min-height:100vh;"
    "display:flex;align-items:center;justify-content:center;"
    "background:#f5f7fa;color:#172b4d;"
    "font-family:Arial,sans-serif;}"
    ".panel{text-align:center;padding:32px;}"
    "h1{font-size:28px;font-weight:600;margin-bottom:8px;}"
    ".subtitle{color:#8492a6;font-size:14px;margin-bottom:36px;}"
    ".switch{position:relative;display:inline-block;"
    "width:100px;height:54px;}"
    ".switch input{opacity:0;width:0;height:0;}"
    ".slider{position:absolute;cursor:pointer;inset:0;"
    "background:#cbd5e1;border-radius:40px;"
    "transition:background .25s;}"
    ".slider:before{content:'';position:absolute;"
    "height:44px;width:44px;left:5px;bottom:5px;"
    "background:white;border-radius:50%;"
    "box-shadow:0 2px 6px #0002;"
    "transition:transform .25s;}"
    "input:checked+.slider{background:#22c55e;}"
    "input:checked+.slider:before{transform:translateX(46px);}"
    ".status{font-size:15px;font-weight:600;"
    "margin-top:20px;color:%s;}"
    "</style>"
    "</head>"
    "<body>"
    "<main class='panel'>"
    "<h1>Smart Switch</h1>"
    "<p class='subtitle'>Home Control</p>"
    "<form action='/' method='post'>"
    "<input type='hidden' name='switch_state' value='OFF'>"
    "<label class='switch'>"
    "<input type='checkbox' name='switch_state' value='ON' "
    "onchange='this.form.submit()' %s>"
    "<span class='slider'></span>"
    "</label>"
    "<div class='status'>%s</div>"
    "</form>"
    "</main>"
    "</body>"
    "</html>";

static esp_err_t send_page(httpd_req_t *req)
{
    char page[2048];

    snprintf(page, sizeof(page), HTML_PAGE,
             switch_is_on ? "#16a34a" : "#64748b",
             switch_is_on ? "checked" : "",
             switch_is_on ? "ON" : "OFF");

    httpd_resp_set_type(req, "text/html; charset=UTF-8");
    return httpd_resp_send(req, page, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t get_handler(httpd_req_t *req)
{
    return send_page(req);
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
        switch_is_on = true;
    } else if (strstr(buf, "switch_state=OFF") != NULL) {
        switch_is_on = false;
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid state");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "user requested switch %s",
             switch_is_on ? "ON" : "OFF");

    if (set_switch_cb != NULL) {
        set_switch_cb(switch_is_on);
    }

    return send_page(req);
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
