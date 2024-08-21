
#include <cstring>
#include "esp_log.h"
#include "wificlient.h"

#define WIFI_CONNECTED_BIT (0x0001)

WifiClient::WifiClient(std::string sid, std::string password)
{
    _sid = sid;
    _password = password;
    _eventGroup = xEventGroupCreate();
    esp_log_level_set("wifi", ESP_LOG_NONE);
}

void WifiClient::eventHandler(esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        _connected = false;
        esp_wifi_connect();
        printf("connecting to wifi\n");
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (_connected)
        {
            _connected = false;
            printf("wifi disconnected, trying to reconnect\n");
        }
        xEventGroupClearBits(_eventGroup, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        _ipinfo = event->ip_info;
        sprintf(_ip, "%d.%d.%d.%d",                 
            (int)((_ipinfo.ip.addr>>0) & 0xFF),
            (int)((_ipinfo.ip.addr>>8) & 0xFF),
            (int)((_ipinfo.ip.addr>>16) & 0xFF),
            (int)((_ipinfo.ip.addr>>24) & 0xFF));
        xEventGroupSetBits(_eventGroup, WIFI_CONNECTED_BIT);

        if (!_connected)
        {
            _connected = true;
            printf("wifi connection to sid %s, ip=%s\n", _sid.c_str(), _ip);
        }
    }
}

WifiClient::~WifiClient()
{
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, _handlerWifi);
    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, _handlerIp);
}

bool WifiClient::waitForConnection(int timeoutMs) const
{
    if (!_connected)
    {
        auto ticks = (timeoutMs == 0) ? portMAX_DELAY : timeoutMs / portTICK_PERIOD_MS;
        xEventGroupWaitBits(_eventGroup, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, ticks);
    }
    return _connected;
}

void WifiClient::stayConnected()
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    _config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&_config));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &WifiClient::eventHandler,
                                                        this,
                                                        &_handlerWifi));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &WifiClient::eventHandler,
                                                        this,
                                                        &_handlerIp));

    std::memset(&_wifiConfig, 0, sizeof(_wifiConfig));
    std::memcpy(_wifiConfig.sta.ssid, _sid.c_str(), std::min(_sid.length(), sizeof(wifi_sta_config_t::ssid)));
    std::memcpy(_wifiConfig.sta.password, _password.c_str(), std::min(_sid.length(), sizeof(wifi_sta_config_t::password)));
    _wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    _wifiConfig.sta.sae_pwe_h2e = WPA3_SAE_PWE_UNSPECIFIED;
     
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &_wifiConfig) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    printf("wifi initialization completed\n");
}