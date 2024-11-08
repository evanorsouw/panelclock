
#include <time.h>
#include <sys/time.h>

#include "timesyncer.h"

void TimeSyncer::updateTask()
{
    struct tm now;   
    getTimestamp(&now);
    
    auto secondsSinceEpoch = mktime(&now);
    if (secondsSinceEpoch < 0)
    {
        printf("timesyncer: rtc time invalid, resetting to epoch\n");
        setTimestamp(1970,1,1,0,0,0,0,0);
        getTimestamp(&now);
        secondsSinceEpoch = mktime(&now);
    }

    struct timeval tv;
    tv.tv_sec = secondsSinceEpoch;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);

    printf("timesyncer: set time from rtc: %04d:%02d:%02d %02d:%02d:%02d\n", 
        now.tm_year + 1900,
        now.tm_mon + 1, 
        now.tm_mday, 
        now.tm_hour, 
        now.tm_min, 
        now.tm_sec);

    vTaskDelay(3600000 / portTICK_PERIOD_MS);
}

void TimeSyncer::setTimestamp(int year, int month, int mday, int wday, int hour, int minutes, int seconds, int millies)
{
    struct tod when;
    when.year = year;
    when.mon = month - 1;
    when.mday = mday;
    when.wday = wday;
    when.hour = hour;
    when.min = minutes;
    when.sec = seconds;
    _rtc.setTime(&when);
}

struct tm * TimeSyncer::getTimestamp(struct tm *when)
{
    _rtc.readTimeFromChip();
    auto nowRtc = _rtc.getTime();

    when->tm_year = nowRtc->year - 1900;
    when->tm_mon = nowRtc->mon;
    when->tm_mday = nowRtc->mday;
    when->tm_hour  = nowRtc->hour;
    when->tm_min  = nowRtc->min;
    when->tm_sec  = nowRtc->sec;
    when->tm_isdst = 0;

    return when;
}
