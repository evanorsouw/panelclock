
#ifndef _APPLICATIONCONTEXT_H_
#define _APPLICATIONCONTEXT_H_

#include <cmath>
#include <numbers>
#include <esp_timer.h>

#include "font.h"
#include "timeinfo.h"

class ApplicationContext
{
private:
    timeinfo _now;
    long _msSinceMidnight;
    float _intensity;
    Font *_fonttimeSmall;
    Font *_fonttimeLarge;
    Font *_fontdate;
    Font *_fontWhiteMagic;
    Font *_fontweatherL;
    Font *_fontweatherS;
    Font *_fontIcons4;
    Font *_fontIcons9;
    Font *_fontIcons18;
    Font *_fontIcons22;

public:
    ApplicationContext()
    {
        _fonttimeSmall = Font::getFont("arial-bold-digits", 9, 11);
        _fonttimeLarge = Font::getFont("arial-bold-digits", 14, 14);
        _fontdate = Font::getFont("arial-rounded-stripped", 11, 11);
        _fontWhiteMagic = Font::getFont("arial-rounded-stripped", 20, 28);
        _fontweatherL = Font::getFont("arial-rounded-stripped", 9, 10);
        _fontweatherS = Font::getFont("arial-rounded-stripped", 7, 7);
        _fontIcons4 = Font::getFont("panelicons", 4, 4);
        _fontIcons9 = Font::getFont("panelicons", 9, 9);
        _fontIcons18 = Font::getFont("panelicons", 18, 18);
        _fontIcons22 = Font::getFont("panelicons", 22, 22);
        _intensity = 1;
    }

    bool timeout(uint64_t timer, int timeoutMs) { return elapsed(timer) > timeoutMs; }
    bool timeoutAndRestart(uint64_t &timer, int timeoutMs) 
    { 
        if (timeout(timer, timeoutMs))
        {
            starttimer(timer);
            return true;
        }
        return false;
    }
    void starttimer(uint64_t &timer) { timer = esp_timer_get_time(); }
    int elapsed(uint64_t timer) { return (int)((esp_timer_get_time() - timer) / 1000); }
    float phase(int cycleMs, bool wave, int offsetMs = 0)
    {
        if (cycleMs == 0)
            return 0.0f;

        float phase = (_msSinceMidnight + offsetMs) % cycleMs / (float)cycleMs;
        if (wave)
        {
            phase = 0.5f + std::cos(phase * (float)std::numbers::pi * 2) / 2.0f;
        }
        return phase;
    }

    void settime(timeinfo now)
    {
        _now = now;
        _msSinceMidnight = ((now.hour() * 60 + now.min()) * 60 + now.sec()) * 1000 + now.millies();
    }
    void intensity(float intensity) { _intensity = intensity; }

    Font *fonttimeLarge() const { return _fonttimeLarge; }
    Font *fonttimeSmall() const { return _fonttimeSmall; }
    Font *fontdate() const { return _fontdate; }
    Font *fontWhiteMagic() const { return _fontWhiteMagic; }
    Font *fontweatherL() const { return _fontweatherL; }
    Font *fontweatherS() const { return _fontweatherS; }
    Font *fontIcons4() const { return _fontIcons4; }
    Font *fontIcons9() const { return _fontIcons9; }
    Font *fontIcons18() const { return _fontIcons18; }
    Font *fontIcons22() const { return _fontIcons22; }


    float intensity() const { return _intensity; }
    timeinfo now() const { return _now; }
    long msSinceMidnight() const { return _msSinceMidnight; }
};

#endif
