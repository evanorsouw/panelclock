
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <esp_timer.h>

#include "application.h"

#define PI M_PI

Application::Application(ApplicationContext &appdata, Graphics &graphics, Environment &env, System &sys, UserInput &userinput)
    : RenderBase(appdata, graphics, env, sys, userinput)
{
    _weatherIntensity = 0.0f;
}

void Application::init()
{
}

void Application::render()
{
    auto now = _system.now();

    if (_system.settings().OnePanel())
    {
        drawClock(1, 1, 48);
        drawTimeOnePanel(now);

        auto g = _graphics.view(4,4,56,56,true);
        g.triangle(-10,-20,20,80,96,120,Color::green);
    }
    else
    {
        drawClock(1, 1, 62);
        drawDateTimeTwoPanel(now);
        drawWeather();
    }
}

bool Application::interact()
{
    if (_userinput.hasKeyDown(UserInput::KEY_BOOT, 0))
    {
        if (_userinput.hasKeyDown(UserInput::KEY_BOOT, 5000))
        {
            auto &settings = _system.settings();
            auto resetsettings = _userinput.hasKeyDown(UserInput::KEY_SET, 0);
            if (resetsettings)
            {
                settings.defaultSettings();
                printf("load default settings\n");
            }
            else
            {
                settings.OnePanel(!settings.OnePanel());
                printf("changed panel size\n");
            }
            settings.saveSettings();
            esp_restart();
        }
    }
    else if (_userinput.hasKeyDown(UserInput::KEY_SET, 1000))
    {
        _userinput.flush();
        return true;    // enter configuration menu
    }
    return false;   // stay in normal clock-mode
}

void Application::drawClock(float x, float y, float diameter)
{
    auto color = Color::white * _appctx.intensity();
    auto colorsecond = Color::red * _appctx.intensity();

    auto cx = x + diameter / 2.0f;
    auto cy = y + diameter / 2.0f;
    // draw 5 minute ticks
    for (int i=0; i< 12; ++i)
    {
        drawLineFromCenter(cx, cy, diameter, i / 12.0f, 0.9f, 1.0f, diameter / 40, color);
    }

    // draw center dot
    _graphics.text(_appctx.fontIcons4(), cx-2, cy+1, "0", color);

    // draw hands
    auto hours = drawtime() / (12 * 3600000.0f);
    drawLineFromCenter(cx, cy, diameter, hours, 0.2f, 0.6f, diameter / 20, color);
    auto minutes = (drawtime() % 3600000) / 3600000.0f;
    drawLineFromCenter(cx, cy, diameter, minutes, 0.2f, 0.8f, diameter / 20, color);
    
    if(_system.settings().SmoothSecondHand())
    {
        auto seconds = (drawtime() % 60000) / 60000.0f;
        drawLineFromCenter(cx, cy, diameter, seconds, 0.1f, 0.9f, diameter / 60, colorsecond);
    }
    else
    {
        auto now = drawtime();
        auto second = (now % 60000) / 1000;
        auto angle = second / 60.f;
        if (_lasttime != second)
        {
            _lasttime = second;
            angle += 0.0015;
        }
        drawLineFromCenter(cx, cy, diameter, angle, 0.1f, 0.9f, diameter / 60, colorsecond);
    }
}

void Application::drawTimeOnePanel(const timeinfo &now)
{
    auto color = Color::white * _appctx.intensity();
    char buf1[20], buf2[20];

    snprintf(buf1, sizeof(buf1), "%02d", now.hour());
    snprintf(buf2, sizeof(buf2), "%02d", now.min());
    auto sizeh = _appctx.fonttimeLarge()->textsize(buf1);
    auto sizem = _appctx.fonttimeLarge()->textsize(buf2);

    auto x = 64 - std::max(sizeh.dx, sizem.dx) - 1;
    auto y = _appctx.fonttimeLarge()->ascend() - 2;
    _graphics.text(_appctx.fonttimeLarge(), x, y, buf1, color);
    y += _appctx.fonttimeLarge()->ascend() - 1;

    _graphics.text(_appctx.fonttimeLarge(), x, y, buf2, color);
    y += _appctx.fonttimeLarge()->ascend() - 2;

    snprintf(buf1, sizeof(buf2), "%02d", now.sec());
    auto size = _appctx.fonttimeSmall()->textsize(buf1);
    x = 64 - size.dx - 1;
    _graphics.text(_appctx.fonttimeSmall(), x, y, buf1, color);
}

void Application::drawDateTimeTwoPanel(const timeinfo &now)
{
    auto color = Color::white * _appctx.intensity();
    char buf[40];

    snprintf(buf, sizeof(buf), "%02d", now.sec());
    auto sizesec = _appctx.fonttimeSmall()->textsize(buf);
    auto xtim = 127 - sizesec.dx - 1;
    _graphics.text(_appctx.fonttimeSmall(),
        xtim, 
        _appctx.fonttimeSmall()->ascend() - 1, 
        buf, color);
    sizesec = _appctx.fonttimeSmall()->textsize("88");

    snprintf(buf, sizeof(buf), "%02d:%02d", now.hour(), now.min());
    auto sizehm = _appctx.fonttimeLarge()->textsize(buf);
    auto ybase = _appctx.fonttimeLarge()->ascend() - 2;
    xtim -= sizehm.dx + 2;
    _graphics.text(_appctx.fonttimeLarge(),
        xtim, 
        ybase, 
        buf, color);

    auto txt = std::string(_system.translate(now.dayOfWeek()));
    txt[0] = std::toupper(txt[0]);
    auto size = _appctx.fontdate()->textsize(txt.c_str());
    ybase += size.dy - 1;
    _graphics.text(_appctx.fontdate(),
        127 - size.dx - 1, 
        ybase, 
        txt.c_str(), color);

    auto month = _system.translate(now.monthName(false));
    snprintf(buf, sizeof(buf), "%d %c%s", now.mday(), std::toupper(month[0]), month+1);
    size = _appctx.fontdate()->textsize(buf);
    ybase += size.dy;
    _graphics.text(_appctx.fontdate(),
        127 - size.dx - 1, 
        ybase, 
        buf, color);
}

void Application::drawWeather()
{
    char buf1[20], buf2[20];

    auto agesec = (_system.now().msticks() - _environment.lastupdate().msticks()) / 1000;        
    if (_environment.valid())
    {
        graduallyUpdateVariable(_weatherIntensity, 1.0f, 0.05f);
    }
    else 
    {
        if (agesec < 15 * 60)
        {
            graduallyUpdateVariable(_weatherIntensity, 0.4f, 0.05f);
        }
        else
        {
            graduallyUpdateVariable(_weatherIntensity, 0.0f, 0.05f);
        }
    }

    auto white = Color::white * (_appctx.intensity() * _weatherIntensity);
    auto lightblue = Color(0x33, 0xcc, 0xff) * (_appctx.intensity() * _weatherIntensity);

    auto windspeed = _environment.windspeed();
    snprintf(buf1, sizeof(buf1), "%dms", (int)(windspeed.value() + 0.5f));
    auto size = _appctx.fontweatherL()->textsize(buf1);
    auto xms = 128 - WeatherImageDx - 1 - size.dx;
    auto ybase = 50.2f;
    if(windspeed.isValid())
    {
        _graphics.text(_appctx.fontweatherL(),
            xms, 
            ybase, 
            buf1, white);
    }

    auto temperature = _environment.temperature();
    snprintf(buf1, sizeof(buf1), "%.1f°", temperature.value());
    auto size1 =_appctx.fontweatherL()->textsize(buf1);
    ybase += _appctx.fontweatherL()->ascend() + 1;

    auto windchill = _environment.windchill();
    snprintf(buf2, sizeof(buf2), "(%.1f)", windchill.value());
    auto size2 = _appctx.fontweatherS()->textsize(buf2);

    if (windchill.isValid())
    {
        _graphics.text(_appctx.fontweatherS(),
            128 - WeatherImageDx - 1 - size2.dx,
            ybase - 2,
            buf2, white);
    }
    if (temperature.isValid())
    {
        _graphics.text(_appctx.fontweatherL(),
            128 - WeatherImageDx - 1 - size2.dx - size1.dx, 
            ybase,
            buf1, white);
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

        _graphics.triangle(x1, y1, x3, y3, x4, y4, lightblue);
        _graphics.triangle(x1, y1, x2, y2, x3, y3, white);
    }
    drawWeatherImage();
}

void Application::drawWeatherImage()
{
    auto weather = _environment.weather();
    if (!weather.isValid())
        return;

    float px = 128 - WeatherImageDx;
    float py = 64 - WeatherImageDy;
    float pdx = WeatherImageDx;
    float pdy = WeatherImageDy;

    auto white = Color::white * (_appctx.intensity() * _weatherIntensity);
    auto darkgray = Color::darkgray * (_appctx.intensity() * _weatherIntensity);
    auto lightgray = Color::lightgray * (_appctx.intensity() * _weatherIntensity);
    auto black = Color::black;
    switch (weather.value())
    {
    default:
        break;
    case weathertype::clouded:      // bewolkt    
        draw2Clouds(px, py, pdx, pdy, white, black);
        break;
    case weathertype::lightning:    // bliksem    
        px = drawCloud(px, py-2, pdx, pdy, white, darkgray);
        drawLightning(px, py, pdx, pdy, white);
        break;
    case weathertype::showers:      // buien    
        px = drawCloud(px, py - 2, pdx, pdy, white, darkgray);
        drawRain(px, py, pdx, pdy, true, white);
        break;
    case weathertype::hail:         // hagel    
        drawCloud(px, py - 2, pdx, pdy, white, darkgray);
        // todo:
        break;
    case weathertype::partlycloudy: // halfbewolkt    
    case weathertype::cloudy:       // lichtbewolkt    
        drawSun(px-4, py-2, pdx, pdy);
        px = drawCloud(px, py, pdx, pdy, white, black);
        break;
    case weathertype::cloudyrain:   // halfbewolkt_regen    
        drawSun(px-4, py-2, pdx, pdy);
        px = drawCloud(px, py, pdx, pdy, white, black);
        drawRain(px, py, pdx, pdy, false, white);
        break;
    case weathertype::clearnight:   // helderenacht    
        drawStars(px, py, pdx, pdy);
        drawMoon(px, py, pdx, pdy, white);
        break;
    case weathertype::fog:          // mist    
        drawCloud(px, py - 3, pdx, pdy, lightgray, darkgray);
        drawFog(px, py, pdx, pdy, white);
        break;
    case weathertype::cloudednight: // nachtbewolkt    
        drawMoon(px, py, pdx, pdy, white);
        drawCloud(px, py, pdx, pdy, white, darkgray);
        break;
    case weathertype::nightfog:     // nachtmist
        drawMoon(px, py, pdx, pdy, lightgray);
        drawFog(px, py, pdx, pdy, white);
        break;
    case weathertype::rain:         // regen    
        px = drawCloud(px, py - 2, pdx, pdy, white, darkgray);
        drawRain(px, py, pdx, pdy, false, white);
        break;
    case weathertype::snow:         // sneeuw
        drawSnow(px, py, pdx, pdy, white);
        break;
    case weathertype::sunny:        // zonnig    
        drawSun(px, py, pdx, pdy);
        break;
    case weathertype::heavyclouds:  // zwaarbewolkt
        draw2Clouds(px, py, pdx, pdy, white, darkgray);
        break;
    }
}

void Application::drawLineFromCenter(float x, float y, float diameter, float index, float l1, float l2, float thickness, Color color)
{
    auto angle = PI * 2.0f * index - PI / 2.0f;
    auto radius = diameter / 2.0f;
    auto o1 = radius * l1;
    auto o2 = radius * l2;
    auto x1 = x + cos(angle) * o1;
    auto y1 = y + sin(angle) * o1;
    auto x2 = x + cos(angle) * o2;
    auto y2 = y + sin(angle) * o2;
    _graphics.line(x1, y1, x2, y2, thickness, color);
}

void Application::drawSun(float x, float y,  float width, float height)
{
    auto size = std::min(width, height);
    auto p1 = phase(20000,false);
    auto p2 = phase(3000,true) * 0.1f + 1.0f;
    auto pw = std::max(1.0f, size / 22.0f);

    auto horizoncolor = Color(226,90,56) * (_appctx.intensity() * _weatherIntensity);
    auto nooncolor = Color(255,242,127) * (_appctx.intensity() * _weatherIntensity);
    auto color = nooncolor;

    if (_environment.sunrise().isValid() && _environment.sunset().isValid())
    {
        auto tnow = (int)(drawtime() / 1000 / 60);
        auto rise = _environment.sunrise().value();
        auto trise = rise.tm_hour * 60 + rise.tm_min;
        auto set = _environment.sunset().value();
        auto tset = set.tm_hour * 60 + set.tm_min;

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
        auto angle = PI / 4 * i + p1 * PI;
        auto dx = max * std::cos(angle);
        auto dy = max * std::sin(angle);
        _graphics.line(cx + dx * 0.65f, cy + dy * 0.65f, cx + dx, cy + dy, pw, color);
    }

    auto s = _appctx.fontIcons9()->textsize("0");
    _graphics.text(_appctx.fontIcons9(), cx - s.dx/2, cy - 1 + s.dy/2, "0", color);
}

void Application::draw2Clouds(float x, float y, float width, float height, Color pen, Color fill)
{
    auto cx = x + phase(21000, true) * (width - 18);
    auto cy = y + (height - 18) / 2 + 18 - 4 + phase(17000, true) * 3;
    _graphics.text(_appctx.fontIcons18(), cx, cy, "B", fill);
    _graphics.text(_appctx.fontIcons18(), cx, cy, "A", pen);

    cx = x + phase(22000, true, 3000) * (width - 22);
    cy = y + (height - 22) / 2 + 22 + 2 + phase(19000, true) * 2;
    _graphics.text(_appctx.fontIcons22(), cx, cy, "B", fill);
    _graphics.text(_appctx.fontIcons22(), cx, cy, "A", pen);
}

float Application::drawCloud(float x, float y, float dx, float dy, Color pen, Color fill)
{
    auto tx = x + phase(21000, true, 3000) * (dx - 22);
    auto ty = y + (dy - 22) / 2 + 22 - 2 + phase(19000, true) * 4;
    _graphics.text(_appctx.fontIcons22(), tx, ty, "B", fill);
    _graphics.text(_appctx.fontIcons22(), tx, ty, "A", pen);
    return tx;
}

void Application::drawLightning(float x, float y, float dx, float dy, Color color)
{
    auto tx = x - 5 + std::rand() % 10;
    auto ty = y + (dy - 18) /2 + 18 + std::rand() % 6;

    if (std::rand() % 1000 < 10)
    {
        _graphics.text(_appctx.fontIcons18(), tx, ty, "F", color);
    }
}

void Application::drawRain(float x, float y, float w, float h, bool heavy, Color color)
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
        _graphics.line(x+x1, y+y1, x+x2, y+y2, pw, color);
        _graphics.line(x+x3, y+y3, x+x4, y+y4, pw, color);
    }
}

void Application::drawMoon(float x, float y, float dx, float dy, Color color)
{
    auto tx = x + (dx - 18) / 2 -3 + 6 * phase(25000, true, 3000);
    auto ty = y + (dy - 22) / 2 + 18 - 2;
    _graphics.text(_appctx.fontIcons18(), tx, ty, "D", color);
}

Color Application::starIntensity(float phase, float when)
{
    float dist = 0.02f;
    auto d = std::abs(phase - when);
    if (d > dist)
        return Color(32,32,32) * _weatherIntensity;
    d = dist - d;
    d = 32 + (d * 223.f / dist);
    return Color(d,d,d) * _weatherIntensity;
}

void Application::drawStars(float x, float y, float dx, float dy)
{
    auto p = phase(20000, false);
    _graphics.set(x + 0.10 * dx, y + 01, starIntensity(p, 0.01));
    _graphics.set(x + 0.40 * dx, y + 03, starIntensity(p, 0.04));
    _graphics.set(x + 0.70 * dx, y + 24, starIntensity(p, 0.02));
    _graphics.set(x + 0.30 * dx, y + 07, starIntensity(p, 0.12));
    _graphics.set(x + 0.90 * dx, y + 13, starIntensity(p, 0.32));
    _graphics.set(x + 0.20 * dx, y + 18, starIntensity(p, 0.48));
    _graphics.set(x + 0.15 * dx, y + 05, starIntensity(p, 0.39));
    _graphics.set(x + 0.60 * dx, y + 17, starIntensity(p, 0.23));
    _graphics.set(x + 0.80 * dx, y + 04, starIntensity(p, 0.78));
    _graphics.set(x + 0.50 * dx, y + 21, starIntensity(p, 0.79));
}

void Application::drawFog(float x, float y, float dx, float dy, Color color)
{
    auto pw = 0.8f;
    auto phase1 = phase(37000, true);
    auto phase2 = phase(19000, true);
    auto y1 = y + (dy - 7) / 2 + 2;
    auto y2 = y1 + 2;
    auto y3 = y2 + 2;
    auto y4 = y3 + 2;
    _graphics.line(x + dx * (0.2f + 0.2f * phase1), y1,   x + dx * (0.7f + 0.05f * phase1), y1, pw, color);
    _graphics.line(x + dx * (0.2f + 0.05f * phase2), y2,  x + dx * (0.55f + 0.1f * phase1), y2, pw, color);
    _graphics.line(x + dx * (0.65f + 0.05f * phase1), y2, x + dx * (0.9f - 0.1f * phase2), y2, pw, color);
    _graphics.line(x + dx * (0.25f - 0.05f * phase1), y3, x + dx * (0.4f + 0.15f * phase2), y3, pw, color);
    _graphics.line(x + dx * (0.45f + 0.25f * phase2), y3, x + dx * (0.95f - 0.2f * phase1), y3, pw, color);
    _graphics.line(x + dx * (0.4f - 0.3f * phase1), y4,   x + dx * (0.75f + 0.05f * phase2), y4, pw, color);
}

void Application::drawSnow(float x, float y, float dx, float dy, Color color)
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
        auto tx = dx * (0.1f + 0.8f * ix / n1 + 0.1f * std::cos(p * PI * (2+(i&1))));
        auto ty = (dy-d) * p;

        if (p > 0.0f && p < 1.0f)
        {
            _graphics.rect(x + tx, y + ty, d, d * (0.3f + 0.7f * phase(1000 + i*400, true)), color);
        }
    }
}