
#include <cmath>
#include <time.h>
#include <sys/time.h>
#include <esp_sntp.h>

#include "settings.h"
#include "timesyncer.h"

TimeSyncer* TimeSyncer::_theone;

TimeSyncer::TimeSyncer(DS3231 &rtc, AppSettings &settings) 
    : _rtc(rtc)
    , _settings(settings)
{
    _theone = this;
    settings.onChanged([this](Setting* setting) { 
        auto update = 
            setting->name() == AppSettings::KeyNTPServer || 
            setting->name() == AppSettings::KeyTZ || 
            setting->name() == AppSettings::KeyNTPInterval;
        if (update)
        {
            initialize(); 
        }
    });
    initialize();
}

TimeSyncer::~TimeSyncer()
{
    esp_sntp_stop();
}

void TimeSyncer::initialize()
{
    setenv("TZ", _settings.TZ().c_str(), 1);
    tzset();

    if (_settings.TimeMode() == 0)
    {
        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, _settings.NTPServer().c_str());

        esp_sntp_set_time_sync_notification_cb(ntp_notification_cb);
        esp_sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
        esp_sntp_set_sync_interval(_settings.NTPInterval() * 1000 * 60);
        esp_sntp_stop();
        esp_sntp_init();

        printf("NTP initialized for server='%s', interval='%d:%02d'\n", 
            _settings.NTPServer().c_str(),
            _settings.NTPInterval() / 60,
            _settings.NTPInterval() % 60);
    }
    else
    {
        printf("NTP disabled\n");
        esp_sntp_stop();
        update(true);   // refresh from RTC
    }
}

void TimeSyncer::update(bool forceRTCUpdate)
{
    if (!forceRTCUpdate && _settings.TimeMode() == 0)
        return; // using NTP

    struct tm now;   
    readUTCFromRTC(&now);

    auto secondsSinceEpoch = timegm(&now);
    if (secondsSinceEpoch < 0)
    {
        printf("timesyncer: rtc time invalid, resetting to epoch\n");
        writeUTCToRTC(1970,1,1,0,0,0,0);
        readUTCFromRTC(&now);
        secondsSinceEpoch = timegm(&now);
    }

    struct timeval tv;
    tv.tv_sec = secondsSinceEpoch;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);

    printf("timesyncer: set time from rtc: %04d-%02d-%02d %02d:%02d:%02d\n", 
        now.tm_year + 1900,
        now.tm_mon + 1, 
        now.tm_mday, 
        now.tm_hour, 
        now.tm_min, 
        now.tm_sec);
}

void TimeSyncer::writeUTCToRTC(int year, int month, int mday, int wday, int hour, int minutes, int seconds)
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

struct tm * TimeSyncer::readUTCFromRTC(struct tm *when)
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

time_t TimeSyncer::timegm(struct tm *tm)
{
    tm->tm_isdst = 0;
    auto t1 = mktime(tm);
    struct tm tm2;
    gmtime_r(&t1, &tm2);
    tm2.tm_isdst = 0;
    auto t2 = mktime(&tm2);
    auto diff = t1 - t2;
    return t1 + diff;
}

void TimeSyncer::ntp_notification_cb(struct timeval *now)
{
    struct tm tm;
    auto rtcsec = _theone->timegm(_theone->readUTCFromRTC(&tm));
    time_t ntpsec = (time_t)now->tv_sec;
    auto diff = std::abs(rtcsec - ntpsec);

    if (diff > 1)
    {
        gmtime_r(&ntpsec, &tm);
        _theone->writeUTCToRTC(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_wday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

    localtime_r(&ntpsec, &tm);
    printf("sntp time: %04d-%02d-%02d %02d:%02d:%02d (diff with rtc=%lld)\n", 
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, diff);
}
