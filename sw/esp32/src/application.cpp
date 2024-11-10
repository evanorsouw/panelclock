
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <esp_timer.h>

#include "application.h"
#include "bootanimations.h"
#include "color.h"
#include "spline.h"

Application::Application(Graphics &graphics, LedPanel &panel, Environment &env, System &sys, UserInput &userinput)
    : _graphics(graphics)
    , _panel(panel)
    , _environment(env)
    , _system(sys)
    , _userinput(userinput)
{
    _iShowScreen = 0;
    _uimode = UIMode::BootAnimation;
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
    _fontIcons4 = Font::getFont("panelicons", 4, 4);
    _fontIcons9 = Font::getFont("panelicons", 9, 9);
    _fontIcons18 = Font::getFont("panelicons", 18, 18);
    _fontIcons22 = Font::getFont("panelicons", 22, 22);

    _bootAnimationPhase = 0;
    _configurationui = new ConfigurationUI(graphics, env, sys, userinput, _fontdate);
}

void Application::renderTask()
{
    Bitmap *screen = 0;
    xQueueReceive(_hRenderQueue, &screen, 1000);

    auto now = _system.now();
    _msSinceMidnight = ((now.hour() * 60 + now.min()) * 60 + now.sec()) * 1000 + now.millies();
     monitorRefreshRate();

    screen->fill(Color::black);

    switch (_uimode)
    {
        case UIMode::BootAnimation:
            runBootAnimation(*screen);
            break;
        case UIMode::DateTime:
            runProduction(*screen, now);
            break;
        case UIMode::Configuration:
            if (_configurationui->render(*screen))
            {
                _system.settings().saveSettings();
                _uimode = UIMode::DateTime;
            }
            break;
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

void Application::runBootAnimation(Bitmap &screen)
{   
    if (_bootAnimationPhase == 0)
    {
        _bootStart = esp_timer_get_time();
        _bootAnimationPhase++;
        _bootAnimations.push_back(new AnimationBackground(_graphics, 0.0f, 0.4f));
        _bootAnimations.push_back(new AnimationDissolveWhiteSquares(_graphics, 0.0f, 1.6f));
        _bootAnimations.push_back(new AnimationColoredSquares(_graphics, 0.0f, 2.0f));
        _bootAnimations.push_back(new AnimationWhiteMagicText(_graphics, _fontWhiteMagic, 2.0f, 3.8f));
        _bootAnimations.push_back(new AnimationWhiteMagicColorShift(_graphics, _fontWhiteMagic, 3.8f, 4.5f));
        _bootAnimations.push_back(new AnimationWhiteMagicRemove(_graphics, _fontWhiteMagic, 4.5f, 5.5f));
    }

    auto busy = false;
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
    if (_bootAnimationPhase == 2)
    {
        _uimode = UIMode::DateTime;
    }
}

void Application::runProduction(Bitmap &screen, const timeinfo &now)
{
    drawClock(screen, 0);
    drawDateTime(screen, now);
    drawWeather(screen);
    productionUserInteraction();
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

    // draw center dot
    _graphics.text(screen, _fontIcons4, cx-2, cy+1, "0", Color::white);

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

void Application::drawWeather(Bitmap &screen)
{
    char buf1[20], buf2[20];

    if (!_environment.valid())
        return;

    auto windspeed = _environment.windspeed();
    sprintf(buf1, "%dms", (int)(windspeed.value() + 0.5f));
    auto size = _fontweatherL->textsize(buf1);
    auto xms = 128 - WeatherImageDx - 1 - size.dx;
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
            128 - WeatherImageDx - 1 - size2.dx,
            ybase - 2,
            buf2, Color::white);
    }
    if (temperature.isValid())
    {
        _graphics.text(screen, _fontweatherL,
            128 - WeatherImageDx - 1 - size2.dx - size1.dx, 
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
        auto angle = windangle.value() + phase(9000,true) * phase(800,true) * 0.1;
        auto x1 = cx + d * 0.5f * std::cos(angle);
        auto y1 = cy + d * 0.5f * std::sin(angle);
        auto x3 = cx - d * 0.25f * std::cos(angle);
        auto y3 = cy - d * 0.25f * std::sin(angle);
        auto x2 = cx - d * 0.5f * std::cos(angle - 0.6f);
        auto y2 = cy - d * 0.5f * std::sin(angle - 0.6f);
        auto x4 = cx - d * 0.5f * std::cos(angle + 0.6f);
        auto y4 = cy - d * 0.5f * std::sin(angle + 0.6f);

        _graphics.triangle(screen, x1, y1, x3, y3, x4, y4, Color(0x33, 0xcc, 0xff));
        _graphics.triangle(screen, x1, y1, x2, y2, x3, y3, Color::white);
    }
    drawWeatherImage(screen);
}

void Application::drawWeatherImage(Bitmap &screen)
{
    auto weather = _environment.weather();
    if (!weather.isValid())
        return;

    float px = _panel.dx() - WeatherImageDx;
    float py = _panel.dy() - WeatherImageDy;
    float pdx = WeatherImageDx;
    float pdy = WeatherImageDy;

    switch (weather.value())
    {
    default:
        break;
    case weathertype::clouded:      // bewolkt    
        draw2Clouds(screen, px, py, pdx, pdy, Color::white, Color::black);
        break;
    case weathertype::lightning:    // bliksem    
        px = drawCloud(screen, px, py-2, pdx, pdy, Color::white, Color::darkgray);
        drawLightning(screen, px, py, pdx, pdy);
        break;
    case weathertype::showers:      // buien    
        px = drawCloud(screen, px, py - 2, pdx, pdy, Color::white, Color::darkgray);
        drawRain(screen, px, py, pdx, pdy, true);
        break;
    case weathertype::hail:         // hagel    
        drawCloud(screen, px, py - 2, pdx, pdy, Color::white, Color::darkgray);
        // todo:
        break;
    case weathertype::partlycloudy: // halfbewolkt    
    case weathertype::cloudy:       // lichtbewolkt    
        drawSun(screen, px-4, py-2, pdx, pdy);
        px = drawCloud(screen, px, py, pdx, pdy, Color::white, Color::black);
        break;
    case weathertype::cloudyrain:   // halfbewolkt_regen    
        drawSun(screen, px-4, py-2, pdx, pdy);
        px = drawCloud(screen, px, py, pdx, pdy, Color::white, Color::black);
        drawRain(screen, px, py, pdx, pdy, false);
        break;
    case weathertype::clearnight:   // helderenacht    
        drawStars(screen, px, py, pdx, pdy);
        drawMoon(screen, px, py, pdx, pdy, Color::white);
        break;
    case weathertype::fog:          // mist    
        drawCloud(screen, px, py - 3, pdx, pdy, Color::lightgray, Color::darkgray);
        drawFog(screen, px, py, pdx, pdy);
        break;
    case weathertype::cloudednight: // nachtbewolkt    
        drawMoon(screen, px, py, pdx, pdy, Color::white);
        drawCloud(screen, px, py, pdx, pdy, Color::white, Color::darkgray);
        break;
    case weathertype::nightfog:     // nachtmist
        drawMoon(screen, px, py, pdx, pdy, Color::lightgray);
        drawFog(screen, px, py, pdx, pdy);
        break;
    case weathertype::rain:         // regen    
        px = drawCloud(screen, px, py - 2, pdx, pdy, Color::white, Color::darkgray);
        drawRain(screen, px, py, pdx, pdy, false);
        break;
    case weathertype::snow:         // sneeuw
        drawSnow(screen, px, py, pdx, pdy);
        break;
    case weathertype::sunny:        // zonnig    
        drawSun(screen, px, py, pdx, pdy);
        break;
    case weathertype::heavyclouds:  // zwaarbewolkt
        draw2Clouds(screen, px, py, pdx, pdy, Color::white, Color::darkgray);
        break;
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

void Application::monitorRefreshRate()
{
    _refreshCount++;
    auto elapsed = drawtime() - _refreshCountStart;
    if (elapsed < 0 || elapsed > 30000)
    {
        _refreshCount = 0;
        _refreshCountStart = drawtime();
    }
    else if (elapsed > 10000)
    {
        printf("%d fps\n", _refreshCount /10);
        _refreshCount = 0;
        _refreshCountStart += 10000;
    }
}

float Application::phase(int cycleMs, bool wave, int offsetMs)
{
    if (cycleMs == 0)
        return 0.0f;

    float phase = (drawtime() + offsetMs) % cycleMs / (float)cycleMs;
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

    auto horizoncolor = Color(226,90,56);
    auto nooncolor = Color(255,242,127);
    auto color = nooncolor;

    if (_environment.sunrise().isValid() && _environment.sunset().isValid())
    {
        auto tnow = (int)(drawtime() / 1000 / 60);
        auto rise = _environment.sunrise().value();
        auto trise = rise.tm_hour * 60 + rise.tm_min;
        auto set = _environment.sunset().value();
        auto tset = set.tm_hour * 60 + set.tm_min;

        //printf("color: tnow=%d, trise=%d, tset=%d\n", tnow, trise, tset);

        if (tnow < trise || tnow > tset)
        {
            color = horizoncolor;
        }
        else 
        {
            auto drise = std::min(120, std::abs(tnow - trise));
            auto dset = std::min(120, std::abs(tset - tnow));
            auto d = std::min(dset, drise) / 120.0;
            color = Color::gradient(horizoncolor, nooncolor, d);
        }
    }
    auto max = size * 0.35f * p2;
    auto cx = x + width / 2;
    auto cy = y + height / 2;
    for (auto i = 0; i < 8; ++i)
    {
        auto angle = (float)std::numbers::pi / 4 * i + p1 * (float)std::numbers::pi;
        auto dx = max * std::cos(angle);
        auto dy = max * std::sin(angle);
        _graphics.line(screen, cx + dx * 0.65f, cy + dy * 0.65f, cx + dx, cy + dy, pw, color);
    }

    auto s = _fontIcons9->textsize("0");
    _graphics.text(screen, _fontIcons9, cx - s.dx/2, cy - 1 + s.dy/2, "0", color);
}

void Application::draw2Clouds(Bitmap &screen, float x, float y, float width, float height, Color pen, Color fill)
{
    auto cx = x + phase(21000, true) * (width - 18);
    auto cy = y + (height - 18) / 2 + 18 - 4;
    _graphics.text(screen, _fontIcons18, cx, cy, "B", fill);
    _graphics.text(screen, _fontIcons18, cx, cy, "A", pen);

    cx = x + phase(22000, true, 3000) * (width - 22);
    cy = y + (height - 22) / 2 + 22 + 2;
    _graphics.text(screen, _fontIcons22, cx, cy, "B", fill);
    _graphics.text(screen, _fontIcons22, cx, cy, "A", pen);
}

float Application::drawCloud(Bitmap &screen, float x, float y, float dx, float dy, Color pen, Color fill)
{
    auto tx = x + phase(21000, true, 3000) * (dx - 22);
    auto ty = y + (dy - 22) / 2 + 22 - 2;
    _graphics.text(screen, _fontIcons22, tx, ty, "B", fill);
    _graphics.text(screen, _fontIcons22, tx, ty, "A", pen);
    return tx;
}

void Application::drawLightning(Bitmap &screen, float x, float y, float dx, float dy)
{
    auto tx = x - 5 + std::rand() % 10;
    auto ty = y + (dy - 18) /2 + 18 + std::rand() % 6;

    if (std::rand() % 1000 < 10)
    {
        _graphics.text(screen, _fontIcons18, tx, ty, "F", Color::white);
    }
}

void Application::drawRain(Bitmap &screen, float x, float y, float w, float h, bool heavy)
{
    auto pw = heavy ? 1.8f : 1.0f;
    auto ph = phase(heavy ? 800 : 2000, false);
    for (auto i = 0; i < (heavy ? 4 : 3); ++i)
    {
        auto xo = (i + 0.5) * (heavy ? 0.17f : 0.22f) * w;
        auto yo = 0.4f * h;
        auto dx = -w * (heavy ? 0.15f : 0.05f);
        auto dy = h - yo - pw;
        ph = (ph + i * 0.2f);
        while (ph > 1.0f) ph -= 1.0f;
        float x1, y1, x2, y2, x3 = 0, y3 = 0, x4 = 0, y4 = 0;
        float length = 0.3f;
        if (ph < length)
        {
            x1 = xo;
            y1 = yo;
            x2 = xo + dx * ph;
            y2 = yo + dy * ph;
            x3 = xo + dx * (ph + 0.5f);
            y3 = yo + dy * (ph + 0.5f);
            x4 = xo + dx * (ph + 0.5f + length);
            y4 = yo + dy * (ph + 0.5f + length);
        }
        else if (ph < (1.0f - length))
        {
            x1 = xo + dx * ph;
            y1 = yo + dy * ph;
            x2 = xo + dx * (ph + length);
            y2 = yo + dy * (ph + length);
        }
        else
        {
            x1 = xo + dx * ph;
            y1 = yo + dy * ph;
            x2 = xo + dx;
            y2 = yo + dy;
            x3 = xo + dx * (ph - 0.5f);
            y3 = yo + dy * (ph - 0.5f);
            x4 = xo + dx * (ph - 0.5f - length);
            y4 = yo + dy * (ph - 0.5f - length);
        }
        _graphics.line(screen, x+x1, y+y1, x+x2, y+y2, pw, Color::white);
        _graphics.line(screen, x+x3, y+y3, x+x4, y+y4, pw, Color::white);
    }
}

void Application::drawMoon(Bitmap &screen, float x, float y, float dx, float dy, Color color)
{
    auto tx = x + (dx - 18) / 2 -3 + 6 * phase(25000, true, 3000);
    auto ty = y + (dy - 22) / 2 + 18 - 2;
    _graphics.text(screen, _fontIcons18, tx, ty, "D", color);
}

Color Application::starIntensity(float phase, float when)
{
    float dist = 0.02f;
    auto d = std::abs(phase - when);
    if (d > dist)
        return Color(32,32,32);
    d = dist - d;
    d = 32 + (d * 223.f / dist);
    return Color(d,d,d);
}

void Application::drawStars(Bitmap &screen, float x, float y, float dx, float dy)
{
    auto p = phase(20000, false);
    screen.set(x + 0.10 * dx, y + 01, starIntensity(p, 0.01));
    screen.set(x + 0.40 * dx, y + 03, starIntensity(p, 0.04));
    screen.set(x + 0.70 * dx, y + 24, starIntensity(p, 0.02));
    screen.set(x + 0.30 * dx, y + 07, starIntensity(p, 0.12));
    screen.set(x + 0.90 * dx, y + 13, starIntensity(p, 0.32));
    screen.set(x + 0.20 * dx, y + 18, starIntensity(p, 0.48));
    screen.set(x + 0.15 * dx, y + 05, starIntensity(p, 0.39));
    screen.set(x + 0.60 * dx, y + 17, starIntensity(p, 0.23));
    screen.set(x + 0.80 * dx, y + 04, starIntensity(p, 0.78));
    screen.set(x + 0.50 * dx, y + 21, starIntensity(p, 0.79));
}

void Application::drawFog(Bitmap &screen, float x, float y, float dx, float dy)
{
    auto pw = 0.8f;
    auto phase1 = phase(37000, true);
    auto phase2 = phase(19000, true);
    auto y1 = y + (dy - 7) / 2 + 2;
    auto y2 = y1 + 2;
    auto y3 = y2 + 2;
    auto y4 = y3 + 2;
    _graphics.line(screen, x + dx * (0.2f + 0.2f * phase1), y1,   x + dx * (0.7f + 0.05f * phase1), y1, pw, Color::white);
    _graphics.line(screen, x + dx * (0.2f + 0.05f * phase2), y2,  x + dx * (0.55f + 0.1f * phase1), y2, pw, Color::white);
    _graphics.line(screen, x + dx * (0.65f + 0.05f * phase1), y2, x + dx * (0.9f - 0.1f * phase2), y2, pw, Color::white);
    _graphics.line(screen, x + dx * (0.25f - 0.05f * phase1), y3, x + dx * (0.4f + 0.15f * phase2), y3, pw, Color::white);
    _graphics.line(screen, x + dx * (0.45f + 0.25f * phase2), y3, x + dx * (0.95f - 0.2f * phase1), y3, pw, Color::white);
    _graphics.line(screen, x + dx * (0.4f - 0.3f * phase1), y4,   x + dx * (0.75f + 0.05f * phase2), y4, pw, Color::white);
}

void Application::drawSnow(Bitmap &screen, float x, float y, float dx, float dy)
{
    auto n1 = 19;
    auto n2 = 10;
    auto ph = phase(10000,false) * n1;
    auto d = dx * 0.05f;
    for (int i = 0; i < n1; ++i)
    {
        auto ix = (i*i) % n1;
        auto low = i;
        auto high = (i + n2) % n1;
        if (low < high && (ph < low || ph > high))
            continue;
        if (high < low && (ph < low && ph > high))
            continue;
        auto p = ph - low;
        if (p < 0)
            p += n1;
        p /= n2;
        auto tx = dx * (0.1f + 0.8f * ix / n1 + 0.1f * std::cos(p * (float)std::numbers::pi * (2+(i&1))));
        auto ty = (dy-d) * p;

        if (p > 0.0f && p < 1.0f)
        {
            _graphics.rect(screen, x + tx, y + ty, d, d * (0.3f + 0.7f * phase(1000 + i*400, true)), Color::white);
        }
    }
}

void Application::productionUserInteraction()
{
    if (_userinput.hasKeyDown(UserInput::KEY_SET, 1000))
    {
        _uimode = UIMode::Configuration;
        _userinput.flush();
        _configurationui->startConfigurationSession();
    }
}
