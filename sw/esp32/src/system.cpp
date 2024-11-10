#include "system.h"

System::System(AppSettings *settings, DS3231 *rtc) 
{
    _settings = settings;
    _rtc = rtc;
    _wifi = nullptr;
    _translator = new Translator(settings->get(settings->KeyLanguage));
    startWifi();
}

System::~System()
{
    delete _translator;
    stopWifi();
}

void System::scanAPs()
{
    _wifi->scanAPs();
}

void System::connectWifi()
{
    _wifi->connect(_settings->WifiSid(), _settings->WifiPassword());
}

timeinfo System::now() const
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    auto dst = _settings->DST();
    if (dst)
    {
        tv.tv_sec += 3600;
    }
    return timeinfo(tv, dst);
}

void System::now(const timeinfo &now)
{
    tod tod;
    tod.year = now.year();
    tod.mon = now.mon();
    tod.mday = now.mday();
    tod.wday = now.wday();
    tod.hour = now.hour();
    tod.min = now.min();
    tod.sec = now.sec();
    _rtc->setTime(&tod);
    
    struct timeval tv;
    tv.tv_sec = mktime((struct tm*)now.tm()) - (now.dst() ? 3600 : 0);
    tv.tv_usec = now.millies() * 1000;
    settimeofday(&tv, nullptr);
}

void System::startWifi()
{
    if (!_wifi)
    {
        _wifi = new WifiClient();
        connectWifi();
    }
}

void System::stopWifi()
{
    if (_wifi)
    {
        delete _wifi;
        _wifi = nullptr;
    }
}
