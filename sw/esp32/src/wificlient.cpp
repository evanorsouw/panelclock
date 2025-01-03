
#include <algorithm>
#include <cstring>
#include <map>
#include "esp_log.h"
#include "wificlient.h"

WifiClient::WifiClient(Event *wifiConnectionEvent)
{
    _sid = "";
    _password = "";
    _ip[0] = 0;
    _wifiConnectionEvent = wifiConnectionEvent;
    _mode = WifiMode::Init;

    _nAccessPoints = 0;
    memset(&_appoints, 0, sizeof(apinfo) * MAXAP);

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
            _wifiConnectionEvent->set();
        } 
        else if (event_id == WIFI_EVENT_SCAN_DONE) 
        {            
            if (_mode == WifiMode::Scanning)
            {
                uint16_t ap_count = 0;
                esp_wifi_scan_get_ap_num(&ap_count);
                ap_count = std::min((uint16_t)MAXAP, ap_count);
                esp_wifi_scan_get_ap_records(&ap_count, _scannedAppoints);
                mergeScannedAccesspoints(ap_count);
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
            snprintf(_ip, sizeof(_ip), "%d.%d.%d.%d",                 
                (int)((_ipinfo.ip.addr>>0) & 0xFF),
                (int)((_ipinfo.ip.addr>>8) & 0xFF),
                (int)((_ipinfo.ip.addr>>16) & 0xFF),
                (int)((_ipinfo.ip.addr>>24) & 0xFF));
            _mode = WifiMode::Connected;
            printf("wifi connection to sid %s, ip=%s\n", _sid.c_str(), _ip);
            _wifiConnectionEvent->set();
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
    _wifiConnectionEvent->wait(timeoutMs);
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

    esp_wifi_disconnect();
    esp_wifi_scan_start(&_scanConfig, false);
    mergeScannedAccesspoints(0);
}

int WifiClient::nAPs() 
{
    std::lock_guard<std::mutex> lock(_apmutex);
    return _nAccessPoints; 
}

const char *WifiClient::APSID(int i) 
{ 
    std::lock_guard<std::mutex> lock(_apmutex);
    return (i >=0 && i< _nAccessPoints) ? _appoints[i].sid : "<empty>"; 
}

void WifiClient::connect(const char *sid, const char *password)
{
    printf("connecting to AP='%s', password='***' (current mode=%d)\n", sid, (int)_mode);

    if (_mode == WifiMode::Connected && _sid == sid && _password == password)
        return;

    if (_mode == WifiMode::Scanning)
    {
        esp_wifi_scan_stop();
    }
      
    _mode = WifiMode::Connecting;    
    _sid = strcmp(sid, "") ? sid : "<<select>>";
    _password = password;

    esp_wifi_stop();
    std::memcpy(_wifiConfig.sta.ssid, _sid.c_str(), std::min(_sid.length(), sizeof(wifi_sta_config_t::ssid)));
    std::memcpy(_wifiConfig.sta.password, _password.c_str(), std::min(_sid.length(), sizeof(wifi_sta_config_t::password)));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &_wifiConfig));
    esp_wifi_start();
}

void WifiClient::initializeWifi()
{

    esp_log_level_set("wifi", ESP_LOG_WARN);
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

void WifiClient::mergeScannedAccesspoints(int count)
{
    std::lock_guard<std::mutex> lock(_apmutex);

    auto now = xTaskGetTickCount();
    auto timeout = CONFIG_FREERTOS_HZ * 120;

    auto istore = 0;
    for (int i=0; i<_nAccessPoints; ++i)
    {
        auto &ap = _appoints[i];
        if (i != istore)
        {
            memcpy(_appoints + istore, _appoints + i, sizeof(apinfo));
        }
        auto age = now - ap.foundAtTicks;
        istore += (age > timeout) ? 0 : 1;
    }
    _nAccessPoints = istore;

    for (auto i=0; i<count; ++i)
    {
        auto sid = (const char *)_scannedAppoints[i].ssid;
        if (!strcmp(sid, ""))
            continue;
    
        for (istore=0; istore < _nAccessPoints; ++istore)
        {
            auto &ap = _appoints[istore];
            if (!strcmp(ap.sid, sid))
                break;
        }
        if (istore < MAXAP)
        {
            auto &ap = _appoints[istore];
            strcpy(ap.sid, sid);
            ap.foundAtTicks = now;
            if (istore >= _nAccessPoints)
            {
                _nAccessPoints++;
            }
        }
    }

    printf("count=%d, istore=%d, accesspoints (%d): ", count, istore, _nAccessPoints);
    for (int i=0; i<_nAccessPoints; ++i)
    {
        printf("'%s' (%ld)' ", _appoints[i].sid, now - _appoints[i].foundAtTicks);
    }
    printf("\n");
}