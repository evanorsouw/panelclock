
#ifndef _APPLICATIONCONTEXT_H_
#define _APPLICATIONCONTEXT_H_

#include <cmath>
#include <numbers>
#include <esp_timer.h>

#include "appsettings.h"
#include "font.h"
#include "timeinfo.h"

class ApplicationContext
{
private:
    AppSettings &_settings;
    timeinfo _now;
    long _msSinceMidnight;
    float _intensity;
    Font *_fonttimeSmall;
    Font *_fonttimeLarge;
    Font *_fontdate;
    Font *_fontWhiteMagic;
    Font *_fontweatherL;
    Font *_fontweatherS;
    Font *_fontIconsXS;
    Font *_fontIconsS;
    Font *_fontIconsL;
    Font *_fontIconsXL;
    Font *_fontSettings;

public:
    ApplicationContext(AppSettings &settings)
        : _settings(settings)
    {
        if (_settings.OnePanel())
        {
            _fonttimeSmall = Font::getFont("arial-bold-digits.ttf", 9, 10);
            _fonttimeLarge = Font::getFont("arial-bold-digits.ttf", 12, 12);
            _fontIconsXS = Font::getFont("panelicons.ttf", 3, 3);
            _fontIconsS = Font::getFont("panelicons.ttf", 7, 7);
            _fontIconsL = Font::getFont("panelicons.ttf", 12, 12);
            _fontIconsXL = Font::getFont("panelicons.ttf", 16, 16);        
            _fontSettings = Font::getFont("fixedfont-condensed.wmf", 0, 0);
        }
        else{
            _fonttimeSmall = Font::getFont("arial-bold-digits.ttf", 9, 11);
            _fonttimeLarge = Font::getFont("arial-bold-digits.ttf", 14, 14);
            _fontIconsXS = Font::getFont("panelicons.ttf", 4, 4);
            _fontIconsS = Font::getFont("panelicons.ttf", 9, 9);
            _fontIconsL = Font::getFont("panelicons.ttf", 18, 18);
            _fontIconsXL = Font::getFont("panelicons.ttf", 22, 22);
            _fontSettings = Font::getFont("fixedfont.wmf", 0, 0);
        }
        _fontdate = Font::getFont("arial-rounded-stripped.ttf", 11, 11);
        _fontWhiteMagic = Font::getFont("arial-rounded-stripped.ttf", 20, 28);
        _fontweatherL = Font::getFont("arial-rounded-stripped.ttf", 9, 10);
        _fontweatherS = Font::getFont("arial-rounded-stripped.ttf", 7, 7);
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
    uint64_t starttimer() { return esp_timer_get_time(); }
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
    Font *fontIconsXS() const { return _fontIconsXS; }
    Font *fontIconsS() const { return _fontIconsS; }
    Font *fontIconsL() const { return _fontIconsL; }
    Font *fontIconsXL() const { return _fontIconsXL; }
    Font *fontSettings() const { return _fontSettings; }
    
    float intensity() const { return _intensity; }
    timeinfo now() const { return _now; }
    long msSinceMidnight() const { return _msSinceMidnight; }
};

#endif
