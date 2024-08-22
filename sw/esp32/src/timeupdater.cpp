
#include <time.h>
#include <sys/time.h>

#include "timeupdater.h"

void TimeUpdater::updateTask()
{
    _rtc.readTimeFromChip();
    auto nowRtc = _rtc.getTime();

    struct tm now;
    now.tm_year = nowRtc->year - 1900;
    now.tm_mon = nowRtc->mon;
    now.tm_mday = nowRtc->mday;
    now.tm_hour  = nowRtc->hour;
    now.tm_min  = nowRtc->min;
    now.tm_sec  = nowRtc->sec;

    struct timeval tv;
    tv.tv_sec = mktime(&now);
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);

    printf("set time from rtc: %04d:%02d:%02d %02d:%02d:%02d\n", now.tm_year, now.tm_mon, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);

    vTaskDelay(3600000 / portTICK_PERIOD_MS);
}