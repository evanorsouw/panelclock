
#ifndef _TIMEINFO_H_
#define _TIMEINFO_H_

#include <time.h>

class  timeinfo
{
private:
    const char *_weekdaysFull[7] = { "monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday" };
    const char *_weekdaysCompact[7] = { "mon", "tue", "wed", "thu", "fri", "sat", "sun" };
    const char *_monthsFull[12] = { "january", "feburary", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december" };
    const char *_monthsCompact[12] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
    struct tm _tm;
    int _millies;

public:
    timeinfo(const timeval &tv)
    {
        _tm = *localtime(&tv.tv_sec);
        _millies = tv.tv_usec / 1000;
    }

    int year() const { return 1900 + _tm.tm_year; }
    int mon() const { return _tm.tm_mon; }      // 0..11
    int yday() const { return _tm.tm_yday; }    // 0..365
    int mday() const { return _tm.tm_mday; }    // 1..31
    int wday() const { return (_tm.tm_wday + 6) % 7; }  // 0 = monday
    int hour() const { return _tm.tm_hour; }
    int min() const { return _tm.tm_min; }
    int sec() const { return _tm.tm_sec; }
    int millies() const { return _millies; }

    const char *dayOfWeek(bool full=true) const { return full ? _weekdaysFull[wday()] : _weekdaysCompact[wday()]; }    
    const char *monthName(bool full=true) const { return full ? _monthsFull[mon()-1] : _monthsCompact[mon()-1] ; }    
};

#endif