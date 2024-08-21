#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <limits>

#include "ds3231.h"
#include "environment.h"
#include "graphics.h"
#include "ledpanel.h"
#include "system.h"

class Application
{
private:
    const char *_timefont = "arial-bold-digits";
    const char *_textfont = "arial-rounded-ascii";
    Graphics &_graphics;
    LedPanel &_panel;
    DS3231 &_rtc;
    Environment &_environment;
    System &_system;
    Bitmap _screen;
    int _iWriteScreen;
    int _refreshCount;
    long _refreshCountStart;

public:
    Application(Graphics &graphics, LedPanel &panel, DS3231 &rtc, Environment &env, System &sys)
        : _graphics(graphics)
        , _panel(panel)
        , _rtc(rtc)
        , _environment(env)
        , _system(sys)
        , _screen(panel.dx(), panel.dy(), 3)
    {
        _iWriteScreen = 0;
        _refreshCountStart = std::numeric_limits<long>::max();
    }

    void foreground();
    void background();

private:
    void drawClock(float x, long msSinceMidnight);
    void drawDateTime(const timeinfo &now);
    void drawIcons();
    void drawWeather();
    void drawCenterLine(float angle, float l1, float l2, float thickness, Color color);
    void sendScreen();
    void swapScreens();
    void monitorRefreshRate(long ms);
};

#endif