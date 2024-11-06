
#include "timeinfo.h"

const int timeinfo::_daysInMonth[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

timeinfo::timeinfo() 
{
    _tm.tm_year = 70;
    _tm.tm_mon = 0;
    _tm.tm_mday = 1;
    _tm.tm_yday = 0;
    _tm.tm_wday = 0;
    _tm.tm_hour = 0;
    _tm.tm_min = 0;
    _tm.tm_sec = 0;
    _tm.tm_isdst = false;
    _millies = 0;
    _msticks = 0;
}

timeinfo::timeinfo(const timeval &tv)
{
    _tm = *localtime(&tv.tv_sec);
    _millies = tv.tv_usec / 1000;
    _msticks = (tv.tv_sec * 1000000 + tv.tv_usec) / 1000;
}

void timeinfo::setDate(int year, int month, int mday)
{
    _tm.tm_year = year - 1900;
    _tm.tm_mon = month;
    _tm.tm_mday = mday;
    set();
}


int timeinfo::daysInMonth(int month, bool leapyear)
{
    if (month < 0 || month > 11)
        return 0;
    return _daysInMonth[month] + ((month == 1 && leapyear) ? 1 : 0); 
}

void timeinfo::set(time_t offsetSeconds)
{
    auto secs = mktime(&_tm) + offsetSeconds;
    _tm = *localtime(&secs);
    _msticks = secs * 1000 + _millies;
}
