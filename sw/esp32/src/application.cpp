
#include "application.h"
#include "color.h"

void Application::renderTask()
{
    Bitmap *screen = 0;
    //printf("fore: waiting for render screen\n");
    xQueueReceive(_hRenderQueue, &screen, 1000);
    //printf("fore: rendering to screen=%p\n", screen);

    auto now = _system.now();
    auto msSinceMidnight = ((now.hour() * 60 + now.min()) * 60 + now.sec()) * 1000 + now.millies();
    monitorRefreshRate(msSinceMidnight);

    screen->fill(Color::black);
    drawClock(*screen, 0, msSinceMidnight);
    drawDateTime(*screen, now);
    drawWeather(*screen);
    drawIcons(*screen);

    //printf("fore: rendering to screen=%p complete\n", screen);
    xQueueSend(_hDisplayQueue, &screen, 1000);
}

void Application::displayTask()
{
    Bitmap *screen;
    xQueueReceive(_hDisplayQueue, &screen, 1000);
    screen->copyTo(_panel, 0, 0);
    showScreenOnPanel();
    xQueueSend(_hRenderQueue, &screen, 1000);
}

void Application::drawClock(Bitmap &screen, float x, long msSinceMidnight)
{
    auto diameter =_panel.dy() - 2;
    auto cx = _panel.dy() / 2.0f;
    auto cy = cx;
    // draw 5 minute ticks
    for (int i=0; i< 12; ++i)
    {
        drawCenterLine(screen, cx, cy, diameter, i / 12.0f, 0.9f, 1.0f, diameter / 40, Color::white);
    }

    // draw hands
    auto hours = msSinceMidnight / (12 * 3600000.0f);
    drawCenterLine(screen, cx, cy, diameter, hours, 0.2f, 0.6f, diameter / 20, Color::white);
    auto minutes = (msSinceMidnight % 3600000) / 3600000.0f;
    drawCenterLine(screen, cx, cy, diameter, minutes, 0.2f, 0.8f, diameter / 20, Color::white);
    auto seconds = (msSinceMidnight % 60000) / 60000.0f;
    drawCenterLine(screen, cx, cy, diameter, seconds, 0.1f, 0.9f, diameter / 60, Color::red);
}

void Application::drawDateTime(Bitmap &screen, const timeinfo &now)
{
    char buf[40];

    _graphics.setfont(_timefont, 8, 11);
    sprintf(buf, "%02d", now.sec());
    auto sizesec = _graphics.textsize(buf);
    _graphics.text(screen, 
        127 - sizesec.dx, 
        sizesec.ybase - 1, 
        buf, Color::white);
    sizesec = _graphics.textsize("88");

    _graphics.setfont(_timefont, 12, 14);
    sprintf(buf, "%02d:%02d", now.hour(), now.min());
    auto sizehm = _graphics.textsize(buf);
    _graphics.text(screen, 
        127 - sizesec.dx - sizehm.dx - 2, 
        sizehm.ybase - 2, 
        buf, Color::white);

    auto txt = std::string(_system.translate(now.dayOfWeek()));
    txt[0] = std::toupper(txt[0]);
    _graphics.setfont(_textfont, 9, 11);
    auto size = _graphics.textsize(txt.c_str());
    _graphics.text(screen, 
        127 - size.dx - 1, 
        20.4, 
        txt.c_str(), Color::white);

    auto month = _system.translate(now.monthName(false));
    sprintf(buf, "%d %c%s", now.mday(), std::toupper(month[0]), month+1);
    _graphics.setfont(_textfont, 9, 11);
    size = _graphics.textsize(buf);
    _graphics.text(screen, 
        127 - size.dx - 1, 
        32.3, 
        buf, Color::white);
}

void Application::drawIcons(Bitmap &screen)
{

}

void Application::drawWeather(Bitmap &screen)
{

}

void Application::drawCenterLine(Bitmap &screen, float x, float y, float diameter, float index, float l1, float l2, float thickness, Color color)
{
    auto angle = M_PI * 2.0f * index - M_PI / 2.0;
    auto radius = diameter / 2.0f;
    auto o1 = radius * l1;
    auto o2 = radius * l2;
    auto x1 = x + cos(angle) * o1;
    auto y1 = y + sin(angle) * o1;
    auto x2 = x + cos(angle) * o2;
    auto y2 = y + sin(angle) * o2;
    _graphics.line(screen, x1, y1, x2, y2, thickness, color);
}

void Application::showScreenOnPanel()
{
    _panel.showScreen(_iShowScreen);
    if (++_iShowScreen == 3)
        _iShowScreen = 0;
    _panel.selectScreen(_iShowScreen);
}

void Application::monitorRefreshRate(long now)
{
    _refreshCount++;
    auto elapsed = now - _refreshCountStart;
    if (elapsed < 0 || elapsed > 30000)
    {
        _refreshCount = 0;
        _refreshCountStart = now;
    }
    else if (elapsed > 10000)
    {
        printf("refreshrate %d fps\n", _refreshCount /10);
        _refreshCount = 0;
        _refreshCountStart += 10000;
    }
}
