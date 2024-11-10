
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
    static const int _daysInMonth[12];
    struct tm _tm;
    int _millies;
    int64_t _msticks;

public:
    timeinfo();
    timeinfo(const timeval &tv, bool isdst=false);

    int year() const { return 1900 + _tm.tm_year; }
    int mon() const { return _tm.tm_mon; }      // 0..11
    int yday() const { return _tm.tm_yday; }    // 0..365
    int mday() const { return _tm.tm_mday; }    // 1..31
    int wday() const { return (_tm.tm_wday + 6) % 7; }  // 0 = monday
    int hour() const { return _tm.tm_hour; }
    int min() const { return _tm.tm_min; }
    int sec() const { return _tm.tm_sec; }
    int millies() const { return _millies; }
    void millies(int value) { _millies = value % 1000; set(); }
    bool dst() const { return _tm.tm_isdst; }
    int64_t msticks() const { return _msticks; }

    void setDate(int year, int month, int mday);
    void hour(int value) { _tm.tm_hour = value % 24; set(); }
    void min(int value) { _tm.tm_min = value % 60; set(); }
    void sec(int value) { _tm.tm_sec = value % 60; set(); }
    void dst(bool dstActive) { _tm.tm_isdst = dstActive ? 1 : 0; set(); }

    void addMinutes(int n) { set(n*60); }
    static int daysInMonth(int month, bool leapyear);

    const char *dayOfWeek(bool full=true) const { return full ? _weekdaysFull[wday()] : _weekdaysCompact[wday()]; }    
    const char *monthName(bool full=true) const { return full ? _monthsFull[mon()] : _monthsCompact[mon()] ; }    

    struct tm *tm() { return &_tm; } 

private:
    void set(time_t offsetSeconds = 0);
};

#endif