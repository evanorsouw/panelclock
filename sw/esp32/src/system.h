
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "appsettings.h"
#include "wificlient.h"
#include "translator.h"
#include "timeinfo.h"

class System
{
private:
    WifiClient *_wifi;
    AppSettings *_settings;
    Translator *_translator;

public:
    System(AppSettings *settings) 
    {
        _settings = settings;
        _wifi = nullptr;
        _translator = new Translator(settings->Language());
        startWifi();
    }

    virtual ~System()
    {
        delete _translator;
        stopWifi();
    }

    bool gotInternet() const { return _wifi->isConnected(); }
    bool waitForInternet(int timeout = 0) const { return _wifi->waitForConnection(timeout); }
    timeinfo now() const
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return timeinfo(tv);
    }

    const char *translate(const char *in) { return _translator->translate(in); }

private:
    void startWifi()
    {
        if (!_wifi)
        {
            _wifi = new WifiClient(
                _settings->WifiSid()->asstring(), 
                _settings->WifiPassword()->asstring());
            _wifi->stayConnected();
        }
    }

    void stopWifi()
    {
        if (_wifi)
        {
            delete _wifi;
            _wifi = nullptr;
        }
    }
};

#endif