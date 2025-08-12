
#include <algorithm>
#include <cstring>
#include "esp_crt_bundle.h"
#include "httpclient.h"

#if 0
  #define LOG(...) printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif


struct bufferinfo
{
    uint8_t *buffer;
    int buffersize;
    int filled;
};

HTTPClient::~HTTPClient()
{
}

esp_err_t HTTPClient::eventHandler(esp_http_client_event_t *evt)
{
    std::function<void(void*,int)> handler = *((std::function<void(void*,int)> *)evt->user_data);
    switch (evt->event_id) 
    {
        case HTTP_EVENT_ERROR:
            LOG("http error\n");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            LOG("http connected\n");
            break;
        case HTTP_EVENT_HEADER_SENT:
            LOG("http header sent\n");
            break;
        case HTTP_EVENT_ON_HEADER:
            LOG("http header key=%s, value=%s\n", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            LOG("http received data %d bytes\n", evt->data_len);
            handler(evt->data, evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            LOG("http finish\n");
            break;
        case HTTP_EVENT_DISCONNECTED:
            LOG("http disconnected\n");
            break;        
        default:
            break;
    }
    return ESP_OK;
}

int HTTPClient::get(const char *url, std::function<void(uint8_t*,size_t)> handler)
{
    esp_http_client_config_t config = { 0 } ;
    config.url = url;
    config.method = HTTP_METHOD_GET;
    config.disable_auto_redirect = true;
    config.event_handler = HTTPClient::eventHandler;
    config.user_data = &handler;

    // play the ssl game but don't check certificates because there is
    // no guaranteed infrastructure to periodically update root certificates.
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    config.cert_pem = nullptr;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) 
    {
        esp_http_client_cleanup(client);
        printf("GET '%s' failed: %s\n", url, esp_err_to_name(err));
        return err;
    }

    auto status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    printf("GET '%s' => STATUS %d\n", url, status);

    return status;
}

int HTTPClient::get(const char *url, uint8_t *buf, size_t bufLen)
{
    int receivedSofar = 0;
    std::function<void(void *,size_t)> handler = [&](void *receivedData, size_t receivedLen) 
    {
         auto tocopy = std::min(bufLen - receivedSofar, receivedLen);
         std::memcpy(buf + receivedSofar, receivedData, tocopy);
         receivedSofar += tocopy;
    };
    return get(url, handler);
}