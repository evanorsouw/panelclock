
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <esp_timer.h>

#include "application.h"
#include "timeinfo.h"
#include "xy.h"

#define PI ((float)std::numbers::pi)

Application::Application(ApplicationContext &appdata, IEnvironment &env, System &sys, UserInput &userinput)
    : RenderBase(appdata, env, sys, userinput)
{
    _weatherIntensity = 0.0f;
    _stripSegments.push_back(dateSegment());
    _stripSegments.push_back(separatorSegment());
    _stripSegments.push_back(temperatureSegment());
    _stripSegments.push_back(separatorSegment());
    _stripSegments.push_back(windangleSegment());
    _stripSegments.push_back(windspeedSegment());
    _stripSegments.push_back(separatorSegment());
}

void Application::init()
{
    _iSegment = 0;
    _segmentOffset = 0;
}

void Application::render(Graphics &graphics)
{
    auto now = _system.now();
    auto agesec = (now.msticks() - _environment.lastUpdate().msticks()) / 1000;
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

    auto mode = _system.settings().PanelMode();
    if (mode == 0) // 1 panel
    {
        drawBackgroundEffects(graphics, now);
        drawSunRiseSet(graphics, 1, 1, 46);
        drawClock(graphics, 1, 1, 46);
        drawTimeOnePanel(graphics, now);
        drawWeatherOnePanel(graphics);
        drawSegments(graphics);
        drawClockFlash(graphics, 1, 1, 46);
    }
    else if (mode == 1) // 2 panel horizontal
    {
        drawBackgroundEffects(graphics, now);
        drawSunRiseSet(graphics, 1, 1, 62);
        drawClock(graphics, 1, 1, 62);
        drawDateTimeHorizontal(graphics, now);
        drawWeatherHorizontal(graphics);
        drawClockFlash(graphics, 1, 1, 62);
    }
    else if (mode == 2) // 2 panel vertical
    {
        drawBackgroundEffects(graphics, now);
        drawDateTimeVertical(graphics, now);
        drawHorizontalAnimatedLine(graphics, 35);
        auto view = graphics.moveOrigin(0, 90, 64, 38);
        drawWeatherVertical(view);
        view = graphics.moveOrigin(0, 37, 64, 64);
        drawSunRiseSet(view, 1, 1, 62);
        drawClock(view, 1, 1, 62);
        drawClockFlash(view, 1, 1, 62);
    }
}

int Application::interact()
{
    if (_userinput.hasKeyDown(UserInput::KEY_DOWN, 3000) && !_userinput.hasKeyDown(UserInput::KEY_UP, 0))
    {
        return 3; // enter configuration menu
    }
    else if (_userinput.hasKeyDown(UserInput::KEY_SET, 1000))
    {
        _userinput.flush();
        return 1; // enter configuration menu
    }
    else if (_userinput.hasKeyDown(UserInput::KEY_UP, 3000) && _userinput.hasKeyDown(UserInput::KEY_DOWN, 3000))
    {
        if (_system.wifiConnected())
            return 2; // OTA firmware upgrade
    }
    return 0; // stay in normal clock-mode
}

void Application::drawSunRiseSet(Graphics &graphics, float x, float y, float diameter)
{
    auto sunrise = _environment.sunrise();
    auto sunset = _environment.sunset();
    if (!sunset.isValid() || !sunrise.isValid())
        return;

    x += diameter / 2.0f;
    y += diameter / 2.0f;
    auto colorSet = scaleWeatherColor(Color(226, 90, 56));
    auto colorRise = scaleWeatherColor(Color(255, 242, 127));

    // auto colorRise = scaleWeatherColor(Color(0, 255, 0)) * 0.7f;
    // auto colorSet = scaleWeatherColor(Color(255, 0, 0)) * 0.7f;

    auto trise = ((sunrise.value().tm_hour % 12) * 60 + sunrise.value().tm_min) / 720.0f;
    auto tset = ((sunset.value().tm_hour % 12) * 60 + sunset.value().tm_min) / 720.0f;

    // trise = (drawtime() % 9000) / 9000.0f;
    // tset = (drawtime() % 8000) / 8000.0f;

    auto dists1 = diameter * 0.35f;
    auto dists2 = diameter * 0.42f;
    auto distr1 = diameter * 0.35f;
    auto distr2 = diameter * 0.42f;
    auto width = diameter * 0.035f;

    auto ar = (0.25f - trise) * std::numbers::pi * 2.0f;
    auto as = (0.25f - tset) * std::numbers::pi * 2.0f;
    if (std::fabs(ar - as) < 0.15f)
    {
        distr1 = diameter * 0.28f;
        distr2 = diameter * 0.35f;
    }

    auto x1 = x + std::cos(ar) * distr1;
    auto y1 = y - std::sin(ar) * distr1;
    auto x2 = x + std::cos(ar) * distr2;
    auto y2 = y - std::sin(ar) * distr2;
    auto x3 = x2 + std::cos(ar + std::numbers::pi / 2) * width;
    auto y3 = y2 - std::sin(ar + std::numbers::pi / 2) * width;
    auto x4 = x2 + std::cos(ar - std::numbers::pi / 2) * width;
    auto y4 = y2 - std::sin(ar - std::numbers::pi / 2) * width;
    graphics.triangle(x1, y1, x3, y3, x4, y4, colorRise);

    x1 = x + std::cos(as) * dists1;
    y1 = y - std::sin(as) * dists1;
    x2 = x + std::cos(as) * dists2;
    y2 = y - std::sin(as) * dists2;
    x3 = x1 + std::cos(as + std::numbers::pi / 2) * width;
    y3 = y1 - std::sin(as + std::numbers::pi / 2) * width;
    x4 = x1 + std::cos(as - std::numbers::pi / 2) * width;
    y4 = y1 - std::sin(as - std::numbers::pi / 2) * width;
    graphics.triangle(x2, y2, x3, y3, x4, y4, colorSet);
}

void Application::drawClock(Graphics &graphics, float x, float y, float diameter)
{
    auto color = Color::white * _appctx.intensity();
    auto colorsecond = Color::red * _appctx.intensity();

    auto cx = x + diameter / 2.0f;
    auto cy = y + diameter / 2.0f;

    // draw 5 minute ticks
    auto animateIntervalMinutes = 5;
    auto animateDuration = 3.0f;

    auto animationinterval = 60000 * animateIntervalMinutes;
    auto duration = animateDuration / 60 / animateIntervalMinutes;
    for (int i = 0; i < 12; ++i)
    {
        auto angle = i / 12.0f;
        auto secnow = (drawtime() % animationinterval) / (float)animationinterval;
        if (secnow < duration)
        {
            auto now = secnow / duration;
            auto start = i / 12.0f;
            auto startmove = start - 0.016f * i;
            if (now > startmove)
            {
                angle = now + (start - startmove);
            }
        }
        else if (secnow < duration * 2)
        {
            auto now = (secnow - duration) / duration;
            auto start = i / 12.0f;
            auto startmove = start - 0.016f * i;
            if (now < startmove)
            {
                angle = now + (start - startmove);
            }
        }

        // fatten tick when second-hand passes
        auto secwall = (drawtime() % 60000) / 1000.0f;
        auto sectick = i * 5;
        auto elapsed = secwall - sectick;
        auto scale = 0.0f;
        if (elapsed > 0.0f && elapsed < 0.4f)
        {
            scale = 0.2f - std::abs(0.2f - elapsed);
        }      

        // draw the tick
        drawLineFromCenter(graphics, 
            cx, cy, diameter, 
            angle,
            0.9f - 0.1f * scale, 
            1.0f + 0.1f * scale, 
            diameter / 40 * (1.0f + scale), 
            color);
    }

    // draw hands
    auto hours = drawtime() / (12 * 3600000.0f);
    drawLineFromCenter(graphics, cx, cy, diameter, hours, 0.2f, 0.6f, diameter / 20, color);
    auto minutes = (drawtime() % 3600000) / 3600000.0f;
    drawLineFromCenter(graphics, cx, cy, diameter, minutes, 0.2f, 0.8f, diameter / 20, color);

    auto angle = 0.0f;
    if (_system.settings().SmoothSecondHand())
    {
        angle = (drawtime() % 60000) / 60000.0f;
    }
    else
    {
        auto now = drawtime();
        auto second = (now % 60000) / 1000;
        angle = second / 60.f;
        if (_lasttime != second)
        {
            _lasttime = second;
            angle += 0.0015;
        }
    }
    drawLineFromCenter(graphics, cx, cy, diameter, angle, -0.25f, -0.1f, diameter / 60, colorsecond);
    drawLineFromCenter(graphics, cx, cy, diameter, angle, 0.1f, 0.9f, diameter / 60, colorsecond);

    // draw center dot
    auto font = _appctx.fontIconsS();
    auto size = font->charsize('2');
    graphics.text(font, cx - size.dx / 2.0f, cy + size.dy / 2.0f - 1, "2", colorsecond);
}

void Application::drawClockFlash(Graphics &graphics, float x, float y, float diameter)
{
    // draw circle animation
    auto sec5interval = (int)(drawtime() / 1000.0f) / 5;
    auto interval = 12 * 3 + 7;
    if ((sec5interval % interval) > 2)
        return;

    auto duration = 2.0f;
    auto elapsed = (drawtime() - sec5interval * 5000) / 1000.0f;
    if (elapsed < duration)
    {
        auto angle = PI * 2.0f * (sec5interval % 12) / 12.0f - PI / 2.0f; 
        auto hd = diameter / 2.0f;       
        auto cx = x + hd + std::cos(angle) * hd * 0.95;
        auto cy = y + hd + std::sin(angle) * hd * 0.95;

        auto col1 = Color(255,255,255);
        auto col2 = Color(255,32,0);
        auto iphase = elapsed / duration;
        auto dphase = 1.0f - iphase;
        auto color = Color::gradient(col1, col2, (iphase * 2));
        auto d1 = std::pow(1 + iphase, 8.0f); 
        auto d2 = std::max(0.0f, d1 - dphase * 10);
        graphics.disc(cx, cy, d1, d2, color);

        auto delay = 0.4f;
        if (elapsed > delay)
        {
            iphase = (elapsed - delay) / duration;
            color = Color::white;
            d1 = std::pow(1.0f + iphase, 12.0f); 
            d2 = std::max(0.0f, d1 - 1.5f);
            graphics.disc(cx, cy, d1, d2, color);
        }
    }
}

void Application::drawTimeOnePanel(Graphics &graphics, const timeinfo &now)
{
    auto color = Color::white * _appctx.intensity();
    char buf1[20], buf2[20];

    snprintf(buf1, sizeof(buf1), "%02d", now.hour());
    snprintf(buf2, sizeof(buf2), "%02d", now.min());
    auto sizeh = _appctx.fonttimeLarge()->textsize(buf1);
    auto sizem = _appctx.fonttimeLarge()->textsize(buf2);

    auto x = 64 - std::max(sizeh.dx, sizem.dx) - 1;
    auto y = _appctx.fonttimeLarge()->ascend() - 2;
    graphics.text(_appctx.fonttimeLarge(), x, y, buf1, color);
    y += _appctx.fonttimeLarge()->ascend() - 1;

    graphics.text(_appctx.fonttimeLarge(), x, y, buf2, color);
    y += _appctx.fonttimeLarge()->ascend() - 2;

    snprintf(buf1, sizeof(buf2), "%02d", now.sec());
    auto size = _appctx.fonttimeSmall()->textsize(buf1);
    x = 64 - size.dx - 1;
    graphics.text(_appctx.fonttimeSmall(), x, y, buf1, color);
}

void Application::drawDateTimeHorizontal(Graphics &graphics, const timeinfo &now)
{
    auto color = Color::white * _appctx.intensity();
    char buf[40];

    snprintf(buf, sizeof(buf), "%02d", now.sec());
    auto sizesec = _appctx.fonttimeSmall()->textsize(buf);
    auto xtim = 127 - sizesec.dx - 1;
    graphics.text(_appctx.fonttimeSmall(),
                  xtim,
                  _appctx.fonttimeSmall()->ascend() - 1,
                  buf, color);
    sizesec = _appctx.fonttimeSmall()->textsize("88");

    snprintf(buf, sizeof(buf), "%02d:%02d", now.hour(), now.min());
    auto sizehm = _appctx.fonttimeLarge()->textsize(buf);
    auto ybase = _appctx.fonttimeLarge()->ascend() - 2;
    xtim -= sizehm.dx + 2;
    graphics.text(_appctx.fonttimeLarge(),
                  xtim,
                  ybase,
                  buf, color);

    auto txt = std::string(_system.translate(now.dayOfWeek()));
    txt[0] = std::toupper(txt[0]);
    auto size = _appctx.fontdate()->textsize(txt.c_str());
    ybase += size.dy - 1;
    graphics.text(_appctx.fontdate(),
                  127 - size.dx - 1,
                  ybase,
                  txt.c_str(), color);

    auto month = _system.translate(now.monthName(false)).c_str();
    snprintf(buf, sizeof(buf), "%d %c%s", now.mday(), std::toupper(month[0]), month + 1);
    size = _appctx.fontdate()->textsize(buf);
    ybase += size.dy;
    graphics.text(_appctx.fontdate(),
                  127 - size.dx - 1,
                  ybase,
                  buf, color);
}

void Application::drawDateTimeVertical(Graphics &graphics, const timeinfo &now)
{
    auto color = Color::white * _appctx.intensity();
    char buf[20];

    auto fontdate = _appctx.fontdate();
    auto wday = _system.translate(now.dayOfWeek());
    auto size = fontdate->textsize(wday);
    auto x = 1 + (62 - size.dx) * phase(90000, true);
    graphics.text(fontdate, x, fontdate->ascend() - 2, wday.c_str(), color);

    auto month = _system.translate(now.monthName(false));
    sprintf(buf, "%d %c%s", now.mday(), std::toupper(month[0]), month.c_str() + 1);
    size = fontdate->textsize(buf);
    x = 1 + (62 - size.dx) * phase(137000, true);
    graphics.text(fontdate, x, fontdate->height() + fontdate->ascend() - 3, buf, color);

    auto fontL = _appctx.fonttimeLarge();
    auto fontS = _appctx.fonttimeSmall();
    sprintf(buf, "%d:%02d", now.hour(), now.min());
    size = fontL->textsize(buf);
    char bufs[10];
    sprintf(bufs, " %02d", now.sec());
    auto sizes = fontS->textsize(bufs);
    x = 1 + (62 - size.dx - sizes.dx) * phase(97000, true);
    graphics.text(fontL, x, fontdate->height() * 2 + fontL->ascend() - 5, buf, color);
    graphics.text(fontS, x + size.dx, fontdate->height() * 2 + fontS->ascend() - 4.5, bufs, color);
}

void Application::drawWeatherHorizontal(Graphics &graphics)
{
    int const WeatherImageDx = 32;
    int const WeatherImageDy = 26;
    char buf1[20], buf2[20];

    auto white = Color::white * (_appctx.intensity() * _weatherIntensity);
    auto lightblue = Color(0x33, 0xcc, 0xff) * (_appctx.intensity() * _weatherIntensity);

    auto windspeed = _environment.windspeed();
    snprintf(buf1, sizeof(buf1), "%dms", (int)(windspeed.value() + 0.5f));
    auto size = _appctx.fontweatherL()->textsize(buf1);
    auto xms = 128 - WeatherImageDx - 1 - size.dx;
    auto ybase = 50.2f;
    if (windspeed.isValid())
    {
        graphics.text(_appctx.fontweatherL(),
                      xms,
                      ybase,
                      buf1, white);
    }

    auto temperature = _environment.temperature();
    snprintf(buf1, sizeof(buf1), "%.1f°", temperature.value());
    auto size1 = _appctx.fontweatherL()->textsize(buf1);
    ybase += _appctx.fontweatherL()->ascend() + 1;

    auto windchill = _environment.windchill();
    snprintf(buf2, sizeof(buf2), "(%.1f)", windchill.value());
    auto size2 = _appctx.fontweatherS()->textsize(buf2);

    if (windchill.isValid())
    {
        graphics.text(_appctx.fontweatherS(),
                      128 - WeatherImageDx - 1 - size2.dx,
                      ybase - 2,
                      buf2, white);
    }
    if (temperature.isValid())
    {
        graphics.text(_appctx.fontweatherL(),
                      128 - WeatherImageDx - 1 - size2.dx - size1.dx,
                      ybase,
                      buf1, white);
    }
    if (_environment.windangle().isValid())
    {
        drawWindArrow(graphics, xms - 13, 36, 14, _environment.windangle().value(), lightblue, white);
    }
    auto view = graphics.moveOrigin(graphics.dx() - WeatherImageDx, graphics.dy() - WeatherImageDy, WeatherImageDx, WeatherImageDy);
    drawWeatherImage(view);
}

void Application::drawWeatherVertical(Graphics &graphics)
{
    int const WeatherImageDx = 32;
    int const WeatherImageDy = 26;

    // graphics.rect(0,0,graphics.dx(), graphics.dy(), Color(20,0,0));

    auto white = Color::white * (_appctx.intensity() * _weatherIntensity);
    auto lightblue = Color(0x33, 0xcc, 0xff) * (_appctx.intensity() * _weatherIntensity);

    char buf[20];
    auto fontL = _appctx.fontweatherL();
    auto fontS = _appctx.fontweatherS();

    auto temperature = _environment.temperature();
    snprintf(buf, sizeof(buf), "%.0f°", temperature.value());
    auto size1 = fontL->textsize(buf);
    if (temperature.isValid())
    {
        graphics.text(fontL, 1, graphics.dy() + fontL->descend(), buf, white);
        auto windchill = _environment.windchill();
        if (windchill.isValid())
        {
            snprintf(buf, sizeof(buf), "(%.0f)", windchill.value());
            graphics.text(fontS, size1.dx + 1, graphics.dy() - fontL->height() + fontS->ascend(), buf, white);
        }
    }

    auto windspeed = _environment.windspeed();
    auto windangle = _environment.windangle();
    if (windangle.isValid() && windspeed.isValid())
    {
        auto arrowsize = 14;
        auto y = graphics.dy() - fontL->height() - fontL->descend();
        drawWindArrow(graphics, 1, y - arrowsize, arrowsize, _environment.windangle().value(), lightblue, white);
        snprintf(buf, sizeof(buf), "%dms", (int)(windspeed.value() + 0.5f));
        graphics.text(fontL, 1 + arrowsize, y - fontL->ascend() - fontL->descend(), buf, white);
    }

    auto view = graphics.moveOrigin(graphics.dx() - WeatherImageDx, graphics.dy() - WeatherImageDy, WeatherImageDx, WeatherImageDy);
    drawWeatherImage(view);
}

void Application::drawWeatherOnePanel(Graphics &graphics)
{
    auto view = graphics.moveOrigin(40, 30, 24, 24);
    drawWeatherImage(view);
}

void Application::drawHorizontalAnimatedLine(Graphics &graphics, float y)
{
    auto offset = 6;
    auto p0 = phase(10000, false, 0);
    for (int x = offset; x < graphics.dx() - offset; x++)
    {
        auto ox = (x - offset) / (float)(graphics.dx() - offset * 2);
        ox += p0;
        if (ox >= 1.0f)
            ox -= 1.0f;
        auto p1 = phase(7000, true, ox * 7000);
        auto p2 = phase(7300, true, (1.0f - ox) * 7300);
        auto o = 10 + p1 * p2 * 245;
        graphics.set(x, y, Color(o, o, o));
    }
}

void Application::drawBackgroundEffects(Graphics &graphics, const timeinfo &now)
{
}


void Application::drawWindArrow(Graphics &graphics, int x, int y, int size, float angle, Color col1, Color col2)
{
    angle = -angle; // y-axis is positive downwards.
    auto pw = size / 21.0f;
    auto d = size - pw - 1.0f;
    auto cx = x + (pw + d) / 2.0f;
    auto cy = y + (pw + d) / 2.0f;
    angle += phase(9000, true) * phase(800, true) * 0.1f;
    auto x1 = cx + d * 0.5f * std::cos(angle);
    auto y1 = cy + d * 0.5f * std::sin(angle);
    auto x3 = cx - d * 0.25f * std::cos(angle);
    auto y3 = cy - d * 0.25f * std::sin(angle);
    auto x2 = cx - d * 0.5f * std::cos(angle - 0.6f);
    auto y2 = cy - d * 0.5f * std::sin(angle - 0.6f);
    auto x4 = cx - d * 0.5f * std::cos(angle + 0.6f);
    auto y4 = cy - d * 0.5f * std::sin(angle + 0.6f);

    graphics.triangle(x1, y1, x3, y3, x4, y4, col1);
    graphics.triangle(x1, y1, x2, y2, x3, y3, col2);
}

void Application::drawWeatherImage(Graphics &graphics)
{
    auto weather = _environment.weather();
    if (!weather.isValid())
        return;

    float x = 0;
    float y = 0;
    float dx = graphics.dx();
    float dy = graphics.dy();
    fxy xy;

    for (auto &layer : weather.value())
    {
        switch (layer.icon)
        {
        case WeatherIcon::CloudWithLightning:
            xy = drawCloud(graphics, x, y, dx, dy, layer);
            drawLightning(graphics, xy.x, xy.y, layer);
            break;
        case WeatherIcon::Cloud:
            drawCloud(graphics, x, y, dx, dy, layer);
            break;
        case WeatherIcon::CloudWithHail:
            xy = drawCloud(graphics, x, y, dx, dy, layer);
            drawHail(graphics, xy.x, xy.y, layer);
            break;
        case WeatherIcon::CloudWithHeavyRain:
            xy = drawCloud(graphics, x, y, dx, dy, layer);
            drawRain(graphics, xy.x, xy.y, true, layer);
            break;
        case WeatherIcon::CloudWithLightRain:
            xy = drawCloud(graphics, x, y, dx, dy, layer);
            drawRain(graphics, xy.x, xy.y, false, layer);
            break;
        case WeatherIcon::SunWithRays:
            drawSun(graphics, x, y, dx, dy, true, layer);
            break;
        case WeatherIcon::SunWithoutRays:
            drawSun(graphics, x, y, dx, dy, false, layer);
            break;
        case WeatherIcon::Moon:
            drawMoon(graphics, x, y, dx, dy, layer);
            break;
        case WeatherIcon::Fog:
            drawFog(graphics, x, y, dx, dy, layer);
            break;
        case WeatherIcon::SnowFlakes:
            drawSnow(graphics, x, y, dx, dy, layer);
            break;
        case WeatherIcon::Stars:
            drawStars(graphics, x, y, dx, dy, layer);
            break;
        }
    }
}

void Application::drawLineFromCenter(Graphics &graphics, float x, float y, float diameter, float index, float l1, float l2, float thickness, Color color)
{
    auto angle = PI * 2.0f * index - PI / 2.0f;
    auto radius = diameter / 2.0f;
    auto o1 = radius * l1;
    auto o2 = radius * l2;
    auto x1 = x + cos(angle) * o1;
    auto y1 = y + sin(angle) * o1;
    auto x2 = x + cos(angle) * o2;
    auto y2 = y + sin(angle) * o2;
    graphics.line(x1, y1, x2, y2, thickness, color);
}

void Application::drawSun(Graphics &graphics, float x, float y, float width, float height, bool drawRays, const WeatherLayer &layer)
{
    x += layer.ox;
    y += layer.oy;
    auto size = std::min(width, height);
    auto p1 = phase(layer.phase1 == 0 ? 20000 : layer.phase1, false);
    auto p2 = phase(layer.phase2 == 0 ? 3000 : layer.phase2, true) * 0.1f + 1.0f;
    auto pw = std::max(1.0f, size / 22.0f);

    auto horizoncolor = scaleWeatherColor(Color(226, 90, 56));
    auto nooncolor = scaleWeatherColor(Color(255, 242, 127));
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
    if (drawRays)
    {
        for (auto i = 0; i < 8; ++i)
        {
            auto angle = PI / 4 * i + p1 * PI;
            auto dx = max * std::cos(angle);
            auto dy = max * std::sin(angle);
            graphics.line(cx + dx * 0.65f, cy + dy * 0.65f, cx + dx, cy + dy, pw, color);
        }
    }
    auto font = _appctx.fontIconsM();
    auto s = font->textsize("0");
    graphics.text(font, cx - s.dx / 2, cy + s.dy / 2 - 1, "0", color);
}

fxy Application::drawCloud(Graphics &graphics, float x, float y, float dx, float dy, const WeatherLayer &layer)
{
    x += layer.ox;
    y += layer.oy;
    auto phase1 = layer.phase1 == 0 ? 21000 : layer.phase1;
    auto phase2 = layer.phase2 == 0 ? 17000 : layer.phase2;
    auto font = getSizedIconFont(layer.fontsize, 'X');

    auto tx = x + phase(phase1, true, layer.phase1offset) * (dx - font->sizex());
    auto ty = y + (dy - font->sizey()) / 2 + font->sizey() - 2 + phase(phase2, true) * 4;
    graphics.text(font, tx, ty, "B", scaleWeatherColor(layer.color1, Color::darkgray));
    graphics.text(font, tx, ty, "A", scaleWeatherColor(layer.color2, Color::white));
    return fxy(tx, ty - font->sizey() * 0.5f);
}

void Application::drawLightning(Graphics &graphics, float x, float y, const WeatherLayer &layer)
{
    x += layer.ox;
    y += layer.oy - 2;
    auto font = getSizedIconFont(layer.fontsize);
    auto color = scaleWeatherColor(layer.color1, Color::white);

    auto wcloud = font->textsize("B").dx;
    auto wlightning = font->textsize("E").dx;
    auto tx = x + std::rand() % ((int)(wcloud - wlightning));
    auto ty = y + (graphics.dy() - font->sizey()) / 2 + font->sizey() + std::rand() % 6;

    if (std::rand() % 1000 < 20)
    {
        graphics.text(font, tx, ty, "E", color);
    }
}

void Application::drawRain(Graphics &graphics, float x, float y, bool heavy, const WeatherLayer &layer)
{
    auto font = getSizedIconFont(layer.fontsize);
    auto color = scaleWeatherColor(layer.color1, Color::white);
    y += layer.ox - 2;
    x += layer.ox + 2;
    auto wcloud = font->textsize("B").dx;
    auto h = graphics.dy() - y;
    auto pw = heavy ? 1.8f : 1.0f;
    auto ph = phase(heavy ? 800 : 2000, false);
    for (auto i = 0; i < (heavy ? 4 : 3); ++i)
    {
        auto xo = (i + 0.5) * (heavy ? 0.22f : 0.30f) * wcloud;
        auto yo = 0.4f * h;
        auto dx = -wcloud * (heavy ? 0.15f : 0.05f);
        auto dy = h - yo - pw;
        ph = (ph + i * 0.2f);
        while (ph > 1.0f)
            ph -= 1.0f;
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
        graphics.line(x + x1, y + y1, x + x2, y + y2, pw, color);
        graphics.line(x + x3, y + y3, x + x4, y + y4, pw, color);
    }
}

void Application::drawHail(Graphics &graphics, float x, float y, const WeatherLayer &layer)
{
    auto font = getSizedIconFont(layer.fontsize, 'X');
    auto color = scaleWeatherColor(layer.color1, Color::white);
    auto wcloud = font->textsize("B").dx;
    auto whail = 2.5f;
    auto dy = graphics.dy() - y;
    auto oph = phase(7000, true) * phase(5300, true, 900);

    for (int i = 0; i < 5; ++i)
    {
        auto ph = phase(800, false, 300 * (i + oph));
        auto hx = x + whail / 2 + (wcloud - whail) * i / 5;
        auto hy = y + std::fmod(ph * dy, dy);
        graphics.rect(hx, hy, whail, whail, color);
    }
}

void Application::drawMoon(Graphics &graphics, float x, float y, float dx, float dy, const WeatherLayer &layer)
{
    auto font = getSizedIconFont(layer.fontsize, 'X');
    auto color = scaleWeatherColor(layer.color1, Color::white * 0.7f);
    y += layer.ox;
    x += layer.ox;
    auto size = font->textsize("D");
    auto tx = x + (dx - size.dx) / 2 - 3 + 6 * phase(layer.phase1 == 0 ? 25000 : layer.phase1, true, layer.phase1offset == 0 ? 3000 : layer.phase1offset);
    auto ty = y + (dy - size.dy) / 2 + size.dy - 2;
    graphics.text(font, tx, ty, "D", color);
}

Color Application::starIntensity(float phase, float when)
{
    float dist = 0.02f;
    auto d = std::abs(phase - when);
    if (d > dist)
        return scaleWeatherColor(Color(32, 32, 32));
    d = dist - d;
    d = 32 + (d * 223.f / dist);
    return scaleWeatherColor(Color(d, d, d));
}

void Application::drawStars(Graphics &graphics, float x, float y, float dx, float dy, const WeatherLayer &layer)
{
    auto p = phase(layer.phase1 == 0 ? 20000 : layer.phase1, false);
    graphics.set(x + 0.10 * dx, y + 01, starIntensity(p, 0.01));
    graphics.set(x + 0.40 * dx, y + 03, starIntensity(p, 0.04));
    graphics.set(x + 0.70 * dx, y + 24, starIntensity(p, 0.02));
    graphics.set(x + 0.30 * dx, y + 07, starIntensity(p, 0.12));
    graphics.set(x + 0.90 * dx, y + 13, starIntensity(p, 0.32));
    graphics.set(x + 0.20 * dx, y + 18, starIntensity(p, 0.48));
    graphics.set(x + 0.15 * dx, y + 05, starIntensity(p, 0.39));
    graphics.set(x + 0.60 * dx, y + 17, starIntensity(p, 0.23));
    graphics.set(x + 0.80 * dx, y + 04, starIntensity(p, 0.78));
    graphics.set(x + 0.50 * dx, y + 21, starIntensity(p, 0.79));
}

void Application::drawFog(Graphics &graphics, float x, float y, float dx, float dy, const WeatherLayer &layer)
{
    auto color = scaleWeatherColor(layer.color1, Color::white);
    auto pw = 0.8f;
    auto phase1 = phase(layer.phase1 == 0 ? 37000 : layer.phase1, true);
    auto phase2 = phase(layer.phase2 == 0 ? 19000 : layer.phase2, true);
    auto y1 = y + (dy - 7) / 2 + 2;
    auto y2 = y1 + 2;
    auto y3 = y2 + 2;
    auto y4 = y3 + 2;
    graphics.line(x + dx * (0.2f + 0.2f * phase1), y1, x + dx * (0.7f + 0.05f * phase1), y1, pw, color);
    graphics.line(x + dx * (0.2f + 0.05f * phase2), y2, x + dx * (0.55f + 0.1f * phase1), y2, pw, color);
    graphics.line(x + dx * (0.65f + 0.05f * phase1), y2, x + dx * (0.9f - 0.1f * phase2), y2, pw, color);
    graphics.line(x + dx * (0.25f - 0.05f * phase1), y3, x + dx * (0.4f + 0.15f * phase2), y3, pw, color);
    graphics.line(x + dx * (0.45f + 0.25f * phase2), y3, x + dx * (0.95f - 0.2f * phase1), y3, pw, color);
    graphics.line(x + dx * (0.4f - 0.3f * phase1), y4, x + dx * (0.75f + 0.05f * phase2), y4, pw, color);
}

void Application::drawSnow(Graphics &graphics, float x, float y, float dx, float dy, const WeatherLayer &layer)
{
    auto color = scaleWeatherColor(layer.color1, Color::white);
    auto n1 = 19;
    auto n2 = 10;
    auto ph = phase(layer.phase1 == 0 ? 10000 : layer.phase1, false) * n1;
    auto d = dx * 0.05f;
    for (int i = 0; i < n1; ++i)
    {
        auto ix = (i * i) % n1;
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
        auto tx = dx * (0.1f + 0.8f * ix / n1 + 0.1f * std::cos(p * PI * (2 + (i & 1))));
        auto ty = (dy - d) * p;

        if (p > 0.0f && p < 1.0f)
        {
            graphics.rect(x + tx, y + ty, d, d * (0.3f + 0.7f * phase((layer.phase2 == 0 ? 1000 : layer.phase2) + i * 400, true)), color);
        }
    }
}

void Application::drawSegments(Graphics &graphics)
{
    auto height = 14;
    auto x = (int)(-_segmentOffset);
    auto offset = -x - _segmentOffset;
    auto i = _iSegment;

    auto widthview = graphics.moveOrigin(0, graphics.dy() - height, graphics.dx(), height);

    while (x < graphics.dx())
    {
        auto &segment = _stripSegments[i];
        auto width = segment.width(widthview);
        auto view = graphics.moveOrigin(x, graphics.dy() - height, width, height);
        segment.render(view, offset);
        x += width;
        if (++i == _stripSegments.size())
        {
            i = 0;
        }
    }

    _segmentOffset += 0.5;
    if (_segmentOffset >= _stripSegments[_iSegment].width(widthview))
    {
        _segmentOffset = 0;
        if (++_iSegment == _stripSegments.size())
        {
            _iSegment = 0;
        }
    }
}

Application::stripsegment Application::temperatureSegment()
{
    auto buf1 = new char[40];
    auto buf2 = new char[40];
    auto font1 = _appctx.fontweatherL();
    auto font2 = _appctx.fontweatherS();
    return stripsegment{
        .width = [this, buf1, buf2, font1, font2](Graphics &graphics)
        { 
            *buf2 = 0;
            if (_environment.temperature().isValid())
            {
                sprintf(buf1, "%.1f°", _environment.temperature().value());
                if (_environment.windchill().isValid())
                {
                    sprintf(buf2, " (%.1f)", _environment.windchill().value());
                }
            }
            else
            {
                sprintf(buf1, "--°");
            }
            return font1->textsize(buf1).dx + font2->textsize(buf2).dx; },
        .render = [this, buf1, buf2, font1, font2](Graphics &graphics, float ox)
        {
            auto white = Color::white * (_appctx.intensity() * _weatherIntensity);
            ox += graphics.text(font1, ox, font1->ascend(), buf1, white);
            graphics.text(font2, ox, font2->ascend() + 1, buf2, white); }};
}

Application::stripsegment Application::windangleSegment()
{
    auto buf = new char[40];
    return stripsegment{
        .width = [this, buf](Graphics &graphics)
        { return graphics.dy() + 1; },
        .render = [this, buf](Graphics &graphics, float ox)
        {
            auto white = Color::white * (_appctx.intensity() * _weatherIntensity);
            auto lightblue = Color(0x33, 0xcc, 0xff) * (_appctx.intensity() * _weatherIntensity);
            drawWindArrow(graphics, 0, 0, graphics.dy(), _environment.windangle().value(),  white, lightblue); }};
}

Application::stripsegment Application::windspeedSegment()
{
    auto buf = new char[40];
    auto font = _appctx.fontweatherL();
    return stripsegment{
        .width = [this, buf, font](Graphics &graphics)
        { 
            if (_environment.windspeed().isValid())
            {
                sprintf(buf, "%.1f ms", _environment.windspeed().value());
            }
            else
            {
                sprintf(buf, "-- ms");
            }
            return font->textsize(buf).dx; },
        .render = [this, buf, font](Graphics &graphics, float ox)
        {
            auto white = Color::white * (_appctx.intensity() * _weatherIntensity);
            graphics.text(font, ox, font->ascend(), buf, white); }};
}

Application::stripsegment Application::dateSegment()
{
    auto buf = new char[40];
    auto font = _appctx.fontweatherL();
    return stripsegment{
        .width = [this, buf, font](Graphics &graphics)
        { 
            auto now = _system.now();   
            sprintf(buf, "%s %d %s %d", 
                _system.translate(now.dayOfWeek(false)).c_str(),
                now.mday(),
                _system.translate(now.monthName(false)).c_str(),
                now.year());
            return font->textsize(buf).dx; },
        .render = [this, buf, font](Graphics &graphics, float ox)
        {
            auto white = Color::white * _appctx.intensity();
            graphics.text(font, ox, font->ascend(), buf, white); }};
}

Application::stripsegment Application::separatorSegment()
{
    return stripsegment{
        .width = [=](Graphics &graphics)
        { return 9; },
        .render = [this](Graphics &graphics, float ox)
        {
            auto darkgray = Color::darkgray * _appctx.intensity();
            graphics.rect(4 + ox, 4, 1, graphics.dy()-8, darkgray); }};
}

Application::stripsegment Application::colorSegment(int dx, Color color)
{
    return stripsegment{
        .width = [=](Graphics &graphics)
        { return dx; },
        .render = [=](Graphics &graphics, float ox)
        { graphics.rect(ox, 0, graphics.dx(), graphics.dy(), color); }};
}

Font *Application::getSizedIconFont(char size, char defaultSize)
{
    size = size == 0 ? defaultSize : size;
    switch (std::tolower(size))
    {
    case 's':
        return _appctx.fontIconsS();
    case 'm':
    default:
        return _appctx.fontIconsM();
    case 'l':
        return _appctx.fontIconsL();
    case 'x':
        return _appctx.fontIconsXL();
    }
}