
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "wificlient.h"

struct timeinfo
{
    int year;
    int mon;
    int mday;
    int hour;
    int min;
    int sec;
    int millies;
};

class System
{
private:
    WifiClient *_wifi;

public:
    System() 
    {
        _wifi = new WifiClient("evopevo_guest", "hereyouare");
        _wifi->stayConnected();
    }

    virtual ~System()
    {
        delete _wifi;
    }

    bool gotInternet() const { return _wifi->isConnected(); }
    bool waitForInternet(int timeout = 0) const { return _wifi->waitForConnection(timeout); }
    timeinfo now() const
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        auto parts = localtime(&tv.tv_sec);

        return timeinfo 
        {
            .year = 1900 + parts->tm_year,
            .mon = parts->tm_mon,
            .mday = parts->tm_mday,
            .hour = parts->tm_hour,
            .min = parts->tm_min,
            .sec = parts->tm_sec,
            .millies = tv.tv_usec / 1000
        };
    }
};

#endif