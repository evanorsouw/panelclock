
#ifndef _WIFI_H_
#define _WIFI_H_

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
//#include "esp_mac.h"

#include "esp_wifi.h"

class WifiClient
{
private:
    wifi_init_config_t _config;
    wifi_config_t _wifiConfig;
    esp_event_handler_instance_t _handlerWifi;
    esp_event_handler_instance_t _handlerIp;
    std::string _sid;
    std::string _password;
    esp_netif_ip_info_t _ipinfo;
    EventGroupHandle_t _eventGroup;
    long _connected;
    char _ip[40];

public:
    WifiClient(std::string sid, std::string password)
    {
        _sid = sid;
        _password = password;
        _eventGroup = xEventGroupCreate();
    }
    virtual ~WifiClient();

    void stayConnected();
    bool isConnected() const { return _connected; }
    const char *ip() const { return _ip; }
    bool waitForConnection(int timeoutMs) const;

private:
    static void eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    { 
        ((WifiClient*)arg)->eventHandler(event_base, event_id, event_data);
    }
    void eventHandler(esp_event_base_t event_base, int32_t event_id, void* event_data);
};

#endif
