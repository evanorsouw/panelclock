
#include <time.h>
#include <math.h>

#include "application.h"
#include "color.h"

void Application::foreground()
{
    auto now = _system.now();

    auto msSinceMidnight = ((now.hour * 60 + now.min) * 60 + now.sec) * 1000 + now.millies;

    _screen.fill(Color::black);
    drawClock(0, msSinceMidnight);

    _screen.copyTo(_panel, 0, 0);
    swapScreens();
}

void Application::background()
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
    
    _system.waitForInternet();
    _environment.update();

    //once every hour
    vTaskDelay(3600 * 1000 / portTICK_PERIOD_MS);
}

void Application::drawClock(float x, long msSinceMidnight)
{
    auto refSize = (float)_panel.dy();
    // draw 5 minute ticks
    for (int i=0; i< 12; ++i)
    {
        drawCenterLine(i / 12.0f, 0.9f, 1.0f, refSize / 40, Color::white);
    }

    // draw hands
    auto hours = msSinceMidnight / (12 * 3600000.0f);
    drawCenterLine(hours, 0.2f, 0.6f, refSize / 20, Color::white);
    auto minutes = (msSinceMidnight % 3600000) / 3600000.0f;
    drawCenterLine(minutes, 0.2f, 0.8f, refSize / 20, Color::white);
    auto seconds = (msSinceMidnight % 60000) / 60000.0f;
    drawCenterLine(seconds, 0.1f, 0.9f, refSize / 60, Color::red);
}

void Application::drawCenterLine(float index, float l1, float l2, float thickness, Color color)
{
    auto angle = M_PI * 2.0f * index - M_PI / 2.0;
    auto hdy = _panel.dy() / 2.0f;
    auto o1 = hdy * l1;
    auto o2 = hdy * l2;
    auto x1 = hdy + cos(angle) * o1;
    auto y1 = hdy + sin(angle) * o1;
    auto x2 = hdy + cos(angle) * o2;
    auto y2 = hdy + sin(angle) * o2;
    _graphics.line(_screen, x1, y1, x2, y2, thickness, color);
}

void Application::swapScreens()
{
    _panel.showScreen(_iVisibleScreen);
    _iVisibleScreen = 1 - _iVisibleScreen;
    _panel.selectScreen(_iVisibleScreen);
}

