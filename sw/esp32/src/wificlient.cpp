
#include <algorithm>
#include <cstring>
#include "esp_log.h"
#include "wificlient.h"

#define WIFI_CONNECTED_BIT (0x0001)

WifiClient::WifiClient()
{
    _sid = "";
    _password = "";
    _ip[0] = 0;
    _eventGroup = xEventGroupCreate();
    _mode = WifiMode::Init;
    _nAccessPoints = 0;

    esp_log_level_set("wifi", ESP_LOG_DEBUG);

    initializeWifi();
}

void WifiClient::eventHandler(esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START) 
        {
            _mode = WifiMode::Connecting;
            ESP_ERROR_CHECK(esp_wifi_connect());
            printf("connecting to access point\n");
        } 
        else if (event_id == WIFI_EVENT_AP_STACONNECTED) 
        {
            auto evt = (wifi_event_ap_staconnected_t*)event_data;
            printf("connected to access point='%s' - MAC=%02X:%02X:%02X:%02X:%02X:%02X\n", 
                _sid.c_str(),
                evt->mac[0], evt->mac[1], evt->mac[2], evt->mac[3], evt->mac[4], evt->mac[5]);
        } 
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED) 
        {
            switch (_mode)
            {
                case WifiMode::Scanning:
                    printf("wifi disconnected\n");
                    break;
                case WifiMode::Connected:
                    printf("wifi disconnected, trying to reconnect\n");
                    esp_wifi_connect();
                    break;
                default:
                    break;
            }
            xEventGroupClearBits(_eventGroup, WIFI_CONNECTED_BIT);
        } 
        else if (event_id == WIFI_EVENT_SCAN_DONE) 
        {            
            uint16_t ap_count = 0;
            esp_wifi_scan_get_ap_num(&ap_count);
            printf("wifi scan complete, found (%d) aps: ", ap_count);

            wifi_ap_record_t aps[16];
            ap_count = std::min((uint16_t)16, ap_count);
            esp_wifi_scan_get_ap_records(&ap_count, aps);

            auto nAP = 0;
            for (auto i=0; i<ap_count && nAP < MAXAP; ++i)
            {
                printf("'%s' ", aps[i].ssid);
                if (aps[i].ssid[0] == 0)
                    continue;
                strcpy(_appoints[nAP++], (const char *)aps[i].ssid);
            }
            _nAccessPoints = nAP;
            printf("\n");
            if (_mode == WifiMode::Scanning)
            {
                scanAPs();
            }
        } 
    }
    if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP) 
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            _ipinfo = event->ip_info;
            sprintf(_ip, "%d.%d.%d.%d",                 
                (int)((_ipinfo.ip.addr>>0) & 0xFF),
                (int)((_ipinfo.ip.addr>>8) & 0xFF),
                (int)((_ipinfo.ip.addr>>16) & 0xFF),
                (int)((_ipinfo.ip.addr>>24) & 0xFF));
            xEventGroupSetBits(_eventGroup, WIFI_CONNECTED_BIT);
            _mode = WifiMode::Connected;
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
    auto ticks = (timeoutMs == 0) ? portMAX_DELAY : timeoutMs / portTICK_PERIOD_MS;
    xEventGroupWaitBits(_eventGroup, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, ticks);
    return _mode == WifiMode::Connected;
}

void WifiClient::scanAPs()
{
    _mode = WifiMode::Scanning;
    printf("start scanning for AP's\n");

    _scanConfig = {0};
    _scanConfig.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    _scanConfig.scan_time.active.min = 1000;
    _scanConfig.scan_time.active.max = 5000;
    _scanConfig.show_hidden = true;    

    ESP_ERROR_CHECK(esp_wifi_scan_start(&_scanConfig, false));
}

void WifiClient::connect(const char *sid, const char *password)
{
    printf("connecting to AP='%s', password='%s' (current mode=%d)\n", sid, password, (int)_mode);

    if (_mode == WifiMode::Scanning)
    {
        ESP_ERROR_CHECK(esp_wifi_scan_stop());
    }

    if (_mode == WifiMode::Connected || _mode == WifiMode::Connecting)
        return;

    _mode = WifiMode::Connecting;    

    _sid = sid;
    _password = password;

    std::memcpy(_wifiConfig.sta.ssid, _sid.c_str(), std::min(_sid.length(), sizeof(wifi_sta_config_t::ssid)));
    std::memcpy(_wifiConfig.sta.password, _password.c_str(), std::min(_sid.length(), sizeof(wifi_sta_config_t::password)));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &_wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_stop() );
    ESP_ERROR_CHECK(esp_wifi_start() );
}

void WifiClient::initializeWifi()
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
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

     std::memset(&_wifiConfig, 0, sizeof(_wifiConfig));
    _wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    _wifiConfig.sta.sae_pwe_h2e = WPA3_SAE_PWE_UNSPECIFIED;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &_wifiConfig));

    printf("wifi initialization completed\n");
}