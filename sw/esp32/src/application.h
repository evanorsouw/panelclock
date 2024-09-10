#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <limits>

#include "animation.h"
#include "bitmap.h"
#include "ds3231.h"
#include "environment.h"
#include "graphics.h"
#include "ledpanel.h"
#include "system.h"

class Application
{
private:
    int const WeatherImageDx = 32;
    int const WeatherImageDy = 26;

    Graphics &_graphics;
    LedPanel &_panel;
    Environment &_environment;
    System &_system;
    QueueHandle_t _hRenderQueue;
    QueueHandle_t _hDisplayQueue;
    Font *_fonttimeLarge;
    Font *_fonttimeSmall;
    Font *_fontdate;
    Font *_fontWhiteMagic;
    Font *_fontweatherL;
    Font *_fontweatherS;
    Font *_fontIcons4;
    Font *_fontIcons8;
    Font *_fontIcons9;
    Font *_fontIcons18;
    Font *_fontIcons22;
    int _refreshCount;
    long _refreshCountStart;
    int _iShowScreen;
    long _msSinceMidnight;
    int _bootAnimationPhase;
    int64_t _bootStart;
    std::vector<Animation*> _bootAnimations;

public:
    Application(Graphics &graphics, LedPanel &panel, Environment &env, System &sys);

    void renderTask();
    void displayTask();

private:
    bool runBootAnimation(Bitmap &screen);
    void drawClock(Bitmap &screen, float x);
    void drawDateTime(Bitmap &screen, const timeinfo &now);
    void drawIcons(Bitmap &screen);
    void drawWeather(Bitmap &screen);
    void drawWeatherImage(Bitmap &screen);
    void drawWindAngle(Bitmap &screen);
    void drawCenterLine(Bitmap &screen, float x, float y, float diameter, float index, float l1, float l2, float thickness, Color color);
    void sendScreen();
    void showScreenOnPanel();
    void monitorRefreshRate(long ms);
    float phase(int ms, bool wave, int offsetms=0);
    void drawSun(Bitmap &screen, float x, float y, float dx, float dy);
    float drawCloud(Bitmap &screen, float x, float y, float dx, float dy, Color line, Color fill);
    void draw2Clouds(Bitmap &screen, float x, float y, float dx, float dy, Color line, Color fill);
    void drawLightning(Bitmap &screen, float x, float y, float dx, float dy);
    void drawRain(Bitmap &screen, float x, float y, float dx, float dy, bool heavy);
    void drawMoon(Bitmap &screen, float x, float y, float dx, float dy, Color color);
    void drawStars(Bitmap &screen, float x, float y, float dx, float dy);
    Color starIntensity(float phase, float when);
    void drawFog(Bitmap &screen, float x, float y, float dx, float dy);
    void drawSnow(Bitmap &screen, float x, float y, float dx, float dy);
    long drawtime() { return _msSinceMidnight; }    
};

#endif