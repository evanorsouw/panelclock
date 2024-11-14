#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <limits>

#include "animation.h"
#include "applicationcontext.h"
#include "bitmap.h"
#include "configurationui.h"
#include "ds3231.h"
#include "environment.h"
#include "graphics.h"
#include "renderbase.h"
#include "system.h"
#include "userinput.h"

class Application : public RenderBase
{    
private:
    int const WeatherImageDx = 32;
    int const WeatherImageDy = 26;

public:
    Application(ApplicationContext &appdata, Graphics &graphics, Environment &env, System &sys, UserInput &userinput);

    void init();
    void render(Bitmap &screen);
    bool interact();
    
private:
    void drawClock(Bitmap &screen, float x);
    void drawDateTime(Bitmap &screen, const timeinfo &now);
    void drawWeather(Bitmap &screen);
    void drawWeatherImage(Bitmap &screen);
    void drawWindAngle(Bitmap &screen);
    void drawLineFromCenter(Bitmap &screen, float x, float y, float diameter, float index, float l1, float l2, float thickness, Color color);
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
};

#endif