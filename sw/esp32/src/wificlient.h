
#ifndef _WIFI_H_
#define _WIFI_H_

#include <string>
#include <map>
#include <mutex>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"

enum class WifiMode { Init, Scanning, Connecting, Connected };

class WifiClient
{
private:
    struct apinfo {
        char sid[33];
        TickType_t foundAtTicks;
    };
private:
    static const int MAXAP = 16;
    wifi_init_config_t _config;
    wifi_config_t _wifiConfig;
    esp_event_handler_instance_t _handlerWifi;
    esp_event_handler_instance_t _handlerIp;
    std::string _sid;
    std::string _password;
    esp_netif_ip_info_t _ipinfo;
    EventGroupHandle_t _eventGroup;
    char _ip[40];
    wifi_scan_config_t _scanConfig;
    wifi_ap_record_t _scannedAppoints[MAXAP];
    apinfo _appoints[MAXAP];
    int _nAccessPoints;
    std::mutex _apmutex;
    WifiMode _mode;

public:
    WifiClient();
    virtual ~WifiClient();
    
    bool isConnecting() const { return _mode == WifiMode::Connecting; }
    bool isConnected() const { return _mode == WifiMode::Connected; }
    bool isScanning() const { return _mode == WifiMode::Scanning; }
    const char *ip() const { return _ip; }
    bool waitForConnection(int timeoutMs) const;

    void scanAPs();
    int nAPs();
    const char *APSID(int i);
    void connect(const char *sid, const char *password);
    

private:
    static void eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    { 
        ((WifiClient*)arg)->eventHandler(event_base, event_id, event_data);
    }
    void eventHandler(esp_event_base_t event_base, int32_t event_id, void* event_data);
    void initializeWifi();
    void mergeScannedAccesspoints(int count);
};

#endif
