
#ifndef _TIMEUPDATER_H_
#define _TIMEUPDATER_H_

#include "ds3231.h"

class TimeUpdater
{
private:
    DS3231 &_rtc;

public:
    TimeUpdater(DS3231 &rtc)
        : _rtc(rtc)
    {}

    void updateTask();
};

#endif