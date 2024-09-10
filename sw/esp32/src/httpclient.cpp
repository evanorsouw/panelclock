
#include <algorithm>
#include <cstring>
#include "esp_crt_bundle.h"
#include "httpclient.h"

struct bufferinfo
{
    uint8_t *buffer;
    int buffersize;
    int filled;
};

esp_err_t HTTPClient::eventHandler(esp_http_client_event_t *evt)
{
    std::function<void(void*,int)> handler = *((std::function<void(void*,int)> *)evt->user_data);
    switch (evt->event_id) 
    {
        case HTTP_EVENT_ERROR:
            //printf("http error\n");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            //printf("http connected\n");
            break;
        case HTTP_EVENT_HEADER_SENT:
            //printf("http header sent\n");
            break;
        case HTTP_EVENT_ON_HEADER:
            //printf("http header key=%s, value=%s\n", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            //printf("http received data %d bytes\n", evt->data_len);
            handler(evt->data, evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            //printf("http finish\n");
            break;
        case HTTP_EVENT_DISCONNECTED:
            //printf("http disconnected\n");
            break;        
        default:
            break;
    }
    return ESP_OK;
}

int HTTPClient::get(const char *url, std::function<void(void*,int)> handler)
{
    esp_http_client_config_t config = { 0 } ;
    config.url = url;
    config.method = HTTP_METHOD_GET;
    config.disable_auto_redirect = true;
    config.event_handler = HTTPClient::eventHandler;
    config.user_data = &handler;

    // play the ssl game but don't check certificates because there is
    // no infrastructure to periodically update root certificates.
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    config.cert_pem = nullptr;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) 
    {
        esp_http_client_cleanup(client);
        printf("GET '%s' failed: %s\n", url, esp_err_to_name(err));
        return -1;
    }

    auto status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    printf("GET '%s' => STATUS %d\n", url, status);

    return status;
}

int HTTPClient::get(const char *url, uint8_t *buf, int bufLen)
{
    int receivedSofar = 0;
    std::function<void(void *,int)> handler = [&](void *receivedData, int receivedLen) 
    {
         auto tocopy = std::min(bufLen - receivedSofar, receivedLen);
         std::memcpy(buf + receivedSofar, receivedData, tocopy);
         receivedSofar += tocopy;
    };
    return get(url, handler);
}