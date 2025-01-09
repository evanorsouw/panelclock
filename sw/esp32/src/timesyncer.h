
#ifndef _TIMEUPDATER_H_
#define _TIMEUPDATER_H_

#include "appsettings.h"
#include "ds3231.h"
#include "events.h"

class TimeSyncer
{
private:
    DS3231& _rtc;
    AppSettings& _settings;
    static TimeSyncer* _theone;
    Event *_updateEvent;
    TimerHandle_t _hTimer;
    const int _updateIntervalTicks = pdMS_TO_TICKS(10 * 60 * 60 * 1000); // every 10 hours

public:
    TimeSyncer(DS3231& rtc, AppSettings& setting, Event *updateEvent);
    virtual ~TimeSyncer();

    Event *triggerUpdateEvent() { return _updateEvent; }
    void triggerUpdate() { _updateEvent->set(); }
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