#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <limits>

#include "bitmap.h"
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
    QueueHandle_t _hRenderQueue;
    QueueHandle_t _hDisplayQueue;
    int _refreshCount;
    long _refreshCountStart;
    int _iShowScreen;

public:
    Application(Graphics &graphics, LedPanel &panel, DS3231 &rtc, Environment &env, System &sys)
        : _graphics(graphics)
        , _panel(panel)
        , _rtc(rtc)
        , _environment(env)
        , _system(sys)
    {
        _iShowScreen = 0;
        _refreshCountStart = std::numeric_limits<long>::max();

        // 2 screen bitmap, 1 being copied, the other for rendering the next one
        _hRenderQueue = xQueueCreate(2, sizeof(Bitmap *));
        auto screen = new Bitmap(panel.dx(), panel.dy(), 3);
        xQueueSend(_hRenderQueue, &screen, 0);
        screen = new Bitmap(panel.dx(), panel.dy(), 3);
        xQueueSend(_hRenderQueue, &screen, 0);

        _hDisplayQueue = xQueueCreate(2, sizeof(Bitmap *));
    }

    void renderTask();
    void displayTask();

private:
    void drawClock(Bitmap &screen, float x, long msSinceMidnight);
    void drawDateTime(Bitmap &screen, const timeinfo &now);
    void drawIcons(Bitmap &screen);
    void drawWeather(Bitmap &screen);
    void drawCenterLine(Bitmap &screen, float x, float y, float diameter, float index, float l1, float l2, float thickness, Color color);
    void sendScreen();
    void showScreenOnPanel();
    void monitorRefreshRate(long ms);
};

#endif