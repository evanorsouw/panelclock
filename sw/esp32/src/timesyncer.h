
#ifndef _TIMEUPDATER_H_
#define _TIMEUPDATER_H_

#include "ds3231.h"

class TimeSyncer
{
private:
    DS3231 &_rtc;

public:
    TimeSyncer(DS3231 &rtc)
        : _rtc(rtc)
    {}

    void updateTask();

private:
    void setTimestamp(int year, int month, int mday, int wday, int hour, int minutes, int seconds, int millies);
    struct tm * getTimestamp(struct tm *when);
};

#endif