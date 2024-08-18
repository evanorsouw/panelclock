#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "ds3231.h"
#include "environment.h"
#include "graphics.h"
#include "ledpanel.h"
#include "system.h"

class Application
{
private:
    Graphics &_graphics;
    LedPanel &_panel;
    DS3231 &_rtc;
    Environment &_environment;
    System &_system;
    Bitmap _screen;
    int _iVisibleScreen;

public:
    Application(Graphics &graphics, LedPanel &panel, DS3231 &rtc, Environment &env, System &sys)
        : _graphics(graphics)
        , _panel(panel)
        , _rtc(rtc)
        , _environment(env)
        , _system(sys)
        , _screen(panel.dx(), panel.dy(), 3)
    {
        _iVisibleScreen = 0;
    }

    void foreground();
    void background();

private:
    void drawClock(float x, long msSinceMidnight);
    void drawCenterLine(float angle, float l1, float l2, float thickness, Color color);
    void sendScreen();
    void swapScreens();
};

#endif