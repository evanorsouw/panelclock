#ifndef _HTTPCLIENT_H_
#define _HTTPCLIENT_H_

#include <functional>
#include "esp_http_client.h"

class HTTPClient
{
public:
    virtual ~HTTPClient();

    int get(const char *url, uint8_t *buf, int bufLen);
    int get(const char *url, std::function<void(uint8_t*,int)> handler);

private:
    static esp_err_t eventHandler(esp_http_client_event_t *evt);
    void handleGetData(void *data, int dataLen);
};

#endif