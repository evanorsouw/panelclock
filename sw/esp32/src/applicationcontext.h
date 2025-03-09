
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
    Font *_fonttimeSmall[3];
    Font *_fonttimeLarge[3];
    Font *_fontdate[3];
    Font *_fontweatherL[3];
    Font *_fontweatherS[3];
    Font *_fontIconsS[3];
    Font *_fontIconsM[3];
    Font *_fontIconsL[3];
    Font *_fontIconsXL[3];
    Font *_fontSettings[3];
    int _activeFontset;

public:
    ApplicationContext(AppSettings &settings)
        : _settings(settings)
    {
        _fonttimeSmall[0] = Font::getFont("ArialRoundBold.ttf", 9, 10);
        _fonttimeLarge[0] = Font::getFont("ArialRoundBold.ttf", 12, 12);
        _fontIconsS[0] = Font::getFont("panelicons.ttf", 3, 3);
        _fontIconsM[0] = Font::getFont("panelicons.ttf", 7, 7);
        _fontIconsL[0] = Font::getFont("panelicons.ttf", 12, 12);
        _fontIconsXL[0] = Font::getFont("panelicons.ttf", 16, 16);        
        _fontSettings[0] = Font::getFont("fixedfont-condensed.wmf", 0, 0);
        _fontdate[0] = Font::getFont("ArialRoundRegular.ttf", 11, 11);
        _fontweatherL[0] = Font::getFont("ArialRoundRegular.ttf", 9, 10);
        _fontweatherS[0] = Font::getFont("ArialRoundRegular.ttf", 7, 7);
        
        _fonttimeSmall[1] = Font::getFont("ArialRoundBold.ttf", 9, 11);
        _fonttimeLarge[1] = Font::getFont("ArialRoundBold.ttf", 14, 14);
        _fontIconsS[1] = Font::getFont("panelicons.ttf", 5, 5);
        _fontIconsM[1] = Font::getFont("panelicons.ttf", 9, 9);
        _fontIconsL[1] = Font::getFont("panelicons.ttf", 18, 18);
        _fontIconsXL[1] = Font::getFont("panelicons.ttf", 22, 22);
        _fontSettings[1] = Font::getFont("fixedfont.wmf", 0, 0);
        _fontdate[1]= Font::getFont("ArialRoundRegular.ttf", 11, 11);
        _fontweatherL[1] = Font::getFont("ArialRoundRegular.ttf", 9, 10);
        _fontweatherS[1] = Font::getFont("ArialRoundRegular.ttf", 7, 7);

        _fonttimeSmall[2] = Font::getFont("ArialRoundBold.ttf", 9, 10);
        _fonttimeLarge[2] = Font::getFont("ArialRoundBold.ttf", 12, 12);
        _fontIconsS[2] = Font::getFont("panelicons.ttf", 5, 5);
        _fontIconsM[2] = Font::getFont("panelicons.ttf", 9, 9);
        _fontIconsL[2] = Font::getFont("panelicons.ttf", 18, 18);
        _fontIconsXL[2] = Font::getFont("panelicons.ttf", 22, 22);
        _fontSettings[2] = Font::getFont("fixedfont-condensed.wmf", 0, 0);
        _fontdate[2] = Font::getFont("ArialRoundRegular.ttf", 11, 11);
        _fontweatherL[2] = Font::getFont("ArialRoundRegular.ttf", 9, 10);
        _fontweatherS[2] = Font::getFont("ArialRoundRegular.ttf", 7, 7);

        auto handler = [&](Setting* _) {
            _activeFontset = _settings.PanelMode();
        };
        _settings.onChanged(handler);
        handler(nullptr);

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

    Font *fonttimeLarge() const { return _fonttimeLarge[_activeFontset]; }
    Font *fonttimeSmall() const { return _fonttimeSmall[_activeFontset]; }
    Font *fontdate() const { return _fontdate[_activeFontset]; }
    Font *fontweatherL() const { return _fontweatherL[_activeFontset]; }
    Font *fontweatherS() const { return _fontweatherS[_activeFontset]; }
    Font *fontIconsS() const { return _fontIconsS[_activeFontset]; }
    Font *fontIconsM() const { return _fontIconsM[_activeFontset]; }
    Font *fontIconsL() const { return _fontIconsL[_activeFontset]; }
    Font *fontIconsXL() const { return _fontIconsXL[_activeFontset]; }
    Font *fontSettings() const { return _fontSettings[_activeFontset]; }
    
    float intensity() const { return _intensity; }
    timeinfo now() const { return _now; }
    long msSinceMidnight() const { return _msSinceMidnight; }
};

#endif
