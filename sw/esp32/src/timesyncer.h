
#ifndef _TIMEUPDATER_H_
#define _TIMEUPDATER_H_

#include "appsettings.h"
#include "ds3231.h"

class TimeSyncer
{
private:
    DS3231& _rtc;
    AppSettings& _settings;
    static TimeSyncer* _theone;

public:
    TimeSyncer(DS3231& rtc, AppSettings& setting);
    virtual ~TimeSyncer();

    /// @brief update local time from RTC if not using NTP
    void update(bool forceRTCUpdate);

private:
    void initialize();
    void writeUTCToRTC(int year, int month, int mday, int wday, int hour, int minutes, int seconds);
    struct tm* readUTCFromRTC(struct tm* when);
    time_t timegm(struct tm* tm);
    static void ntp_notification_cb(struct timeval* now);
};

#endif