
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "appsettings.h"
#include "ds3231.h"
#include "wificlient.h"
#include "translator.h"
#include "timeinfo.h"

class System
{
private:
    WifiClient *_wifi;
    AppSettings *_settings;
    DS3231 *_rtc;
    Translator *_translator;

public:
    System(AppSettings *settings, DS3231 *rtc);
    virtual ~System();

    void scanAPs();
    void connectWifi();
    int nAPs() const { return _wifi->nAPs(); }
    const char *APSID(int i) const { return _wifi->APSID(i); }

    bool wifiConnecting() const { return _wifi->isConnecting(); }
    bool wifiConnected() const { return _wifi->isConnected(); }
    bool wifiScanning() const { return _wifi->isScanning(); }

    bool waitForInternet(int timeout = 0) const { return _wifi->waitForConnection(timeout); }
    const char *IPAddress() const { return _wifi->ip(); }
    timeinfo now() const;
    void now(timeinfo &now);
    const char *translate(const char *in) { return _translator->translate(in); }
    AppSettings &settings() const { return *_settings; }

private:
    void startWifi();
    void stopWifi();
};

#endif