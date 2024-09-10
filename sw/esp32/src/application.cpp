
#include <algorithm>
#include <cmath>
#include <numbers>
#include <esp_timer.h>

#include "application.h"
#include "bootanimations.h"
#include "color.h"
#include "spline.h"

Application::Application(Graphics &graphics, LedPanel &panel, Environment &env, System &sys)
    : _graphics(graphics)
    , _panel(panel)
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

    _fonttimeSmall = Font::getFont("arial-bold-digits", 9, 11);
    _fonttimeLarge = Font::getFont("arial-bold-digits", 14, 14);
    _fontdate = Font::getFont("arial-rounded-stripped", 11, 11);
    _fontWhiteMagic = Font::getFont("arial-rounded-stripped", 20, 28);
    _fontweatherL = Font::getFont("arial-rounded-stripped", 9, 10);
    _fontweatherS = Font::getFont("arial-rounded-stripped", 7, 7);
    _fontIcons8 = Font::getFont("panelicons", 8, 8);
    _fontIcons9 = Font::getFont("panelicons", 9, 9);

    _bootAnimationPhase = 0;
}

void Application::renderTask()
{
    Bitmap *screen = 0;
    xQueueReceive(_hRenderQueue, &screen, 1000);

    auto now = _system.now();
    _msSinceMidnight = ((now.hour() * 60 + now.min()) * 60 + now.sec()) * 1000 + now.millies();
    monitorRefreshRate(_msSinceMidnight);

    screen->fill(Color::black);

    if (!runBootAnimation(*screen))
    {
        drawClock(*screen, 0);
        drawDateTime(*screen, now);
        drawWeather(*screen);
        drawIcons(*screen);
    }
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

bool Application::runBootAnimation(Bitmap &screen)
{   
    auto busy = false;

    if (_bootAnimationPhase == 0)
    {
        _bootStart = esp_timer_get_time();
        _bootAnimationPhase++;
        _bootAnimations.push_back(new AnimationBackground(_graphics, 0.0f, 0.4f));
        _bootAnimations.push_back(new AnimationDissolveWhiteSquares(_graphics, 0.0f, 1.6f));
        _bootAnimations.push_back(new AnimationColoredSquares(_graphics, 0.0f, 2.0f));
        _bootAnimations.push_back(new AnimationWhiteMagicText(_graphics, _fontWhiteMagic, 2.0f, 3.8f));
        _bootAnimations.push_back(new AnimationWhiteMagicColorShift(_graphics, _fontWhiteMagic, 3.8f, 5.0f));
        _bootAnimations.push_back(new Animation(_graphics, 0.0f, 8.0f));
    }

    float when = (esp_timer_get_time() - _bootStart) / 1000000.0f;
    if (_bootAnimationPhase == 1)
    {
        for (auto it : _bootAnimations) 
        {
             busy |= it->run(screen, when); 
        }
        if (!busy)
        {
            while (!_bootAnimations.empty())
            {
                delete _bootAnimations.front();
                _bootAnimations.erase(_bootAnimations.begin());
            }
            _bootAnimationPhase = 2;
        }
    }
    return _bootAnimationPhase < 2;
}

void Application::drawClock(Bitmap &screen, float x)
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
    auto hours = drawtime() / (12 * 3600000.0f);
    drawCenterLine(screen, cx, cy, diameter, hours, 0.2f, 0.6f, diameter / 20, Color::white);
    auto minutes = (drawtime() % 3600000) / 3600000.0f;
    drawCenterLine(screen, cx, cy, diameter, minutes, 0.2f, 0.8f, diameter / 20, Color::white);

    auto seconds = (drawtime() % 60000) / 60000.0f;
    drawCenterLine(screen, cx, cy, diameter, seconds, 0.1f, 0.9f, diameter / 60, Color::red);
}

void Application::drawDateTime(Bitmap &screen, const timeinfo &now)
{
    char buf[40];

    sprintf(buf, "%02d", now.sec());
    auto sizesec = _fonttimeSmall->textsize(buf);
    auto xtim = 127 - sizesec.dx - 1;
    _graphics.text(screen, _fonttimeSmall,
        xtim, 
        _fonttimeSmall->ascend() - 1, 
        buf, Color::white);
    sizesec = _fonttimeSmall->textsize("88");

    sprintf(buf, "%02d:%02d", now.hour(), now.min());
    auto sizehm = _fonttimeLarge->textsize(buf);
    auto ybase = _fonttimeLarge->ascend() - 2;
    xtim -= sizehm.dx + 2;
    _graphics.text(screen, _fonttimeLarge,
        xtim, 
        ybase, 
        buf, Color::white);

    auto txt = std::string(_system.translate(now.dayOfWeek()));
    txt[0] = std::toupper(txt[0]);
    auto size = _fontdate->textsize(txt.c_str());
    ybase += size.dy - 1;
    _graphics.text(screen, _fontdate,
        127 - size.dx - 1, 
        ybase, 
        txt.c_str(), Color::white);

    auto month = _system.translate(now.monthName(false));
    sprintf(buf, "%d %c%s", now.mday(), std::toupper(month[0]), month+1);
    size = _fontdate->textsize(buf);
    ybase += size.dy;
    _graphics.text(screen, _fontdate,
        127 - size.dx - 1, 
        ybase, 
        buf, Color::white);
}

void Application::drawIcons(Bitmap &screen)
{
    if (!_system.gotInternet())
    {
        _graphics.text(screen, _fontIcons9, 60, 8, "G", Color::red);
    }
}

void Application::drawWeather(Bitmap &screen)
{
    auto const imagedx = 32;
    auto const imagedy = 26;
    char buf1[20], buf2[20];

    auto windspeed = _environment.windspeed();
    sprintf(buf1, "%dms", (int)(windspeed.value() + 0.5f));
    auto size = _fontweatherL->textsize(buf1);
    auto xms = 128 - imagedx - 1 - size.dx;
    auto ybase = 50.2f;
    if(windspeed.isValid())
    {
        _graphics.text(screen, _fontweatherL,
            xms, 
            ybase, 
            buf1, Color::white);
    }

    auto temperature = _environment.temperature();
    sprintf(buf1, "%.1f°", temperature.value());
    auto size1 =_fontweatherL->textsize(buf1);
    ybase += _fontweatherL->ascend() + 1;

    auto windchill = _environment.windchill();
    sprintf(buf2, "(%.1f)", windchill.value());
    auto size2 = _fontweatherS->textsize(buf2);

    if (windchill.isValid())
    {
        _graphics.text(screen, _fontweatherS,
            128 - imagedx - 1 - size2.dx,
            ybase - 2,
            buf2, Color::white);
    }
    if (temperature.isValid())
    {
        _graphics.text(screen, _fontweatherL,
            128 - imagedx - 1 - size2.dx - size1.dx, 
            ybase,
            buf1, Color::white);
    }
    auto arrowsize = 14;
    auto x = xms - arrowsize;
    auto y = 36;

    auto windangle = _environment.windangle();
    if (windangle.isValid())
    {
        auto pw = arrowsize / 21.0f;
        auto d = arrowsize - pw - 1.0f;
        auto cx = x + (pw + d) / 2.0f;
        auto cy = y + (pw + d) / 2.0f;
        auto angle = windangle.value() * (float)std::numbers::pi * 2.0f + phase(9000,true) * phase(800,true) * 0.1;
        auto x1 = cx - d * 0.5f * std::cos(angle);
        auto y1 = cy - d * 0.5f * std::sin(angle);
        auto x3 = cx + d * 0.25f * std::cos(angle);
        auto y3 = cy + d * 0.25f * std::sin(angle);
        auto x2 = cx + d * 0.5f * std::cos(angle - 0.6f);
        auto y2 = cy + d * 0.5f * std::sin(angle - 0.6f);
        auto x4 = cx + d * 0.5f * std::cos(angle + 0.6f);
        auto y4 = cy + d * 0.5f * std::sin(angle + 0.6f);

        _graphics.triangle(screen, x1, y1, x3, y3, x4, y4, Color(0x33, 0xcc, 0xff));
        _graphics.triangle(screen, x1, y1, x2, y2, x3, y3, Color::white);
    }
    auto weather = _environment.weather();
    if (weather.isValid())
    {
        drawSun(screen, _panel.dx() - imagedx, _panel.dy() - imagedy, imagedx, imagedy);
    }
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

float Application::phase(float ms, bool wave)
{
    if (ms == 0)
        return 0.0f;

    float phase = drawtime() % (int)ms / (float)ms;
    if (wave)
    {
        phase = 0.5f + std::cos(phase * (float)std::numbers::pi * 2) / 2.0f;
    }
    return phase;
}

void Application::drawSun(Bitmap &screen, float x, float y,  float width, float height)
{
    auto size = std::min(width, height);
    auto p1 = phase(20000,false);
    auto p2 = phase(3000,true) * 0.1f + 1.0f;
    auto pw = std::max(1.0f, size / 22.0f);

    auto yellowgold = Color(250,250,210);
    auto max = size * 0.35f * p2;
    auto cx = x + width / 2;
    auto cy = y + height / 2;
    for (auto i = 0; i < 8; ++i)
    {
        auto angle = (float)std::numbers::pi / 4 * i + p1 * (float)std::numbers::pi;
        auto dx = max * std::cos(angle);
        auto dy = max * std::sin(angle);
        _graphics.line(screen, cx + dx * 0.65f, cy + dy * 0.65f, cx + dx, cy + dy, pw, yellowgold);
    }

    auto s = _fontIcons9->textsize("0");
    _graphics.text(screen, _fontIcons9, cx - s.dx/2, cy - 1 + s.dy/2, "0", yellowgold);
}