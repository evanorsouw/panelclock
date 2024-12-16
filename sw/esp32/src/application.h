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
    struct stripsegment 
    {
        std::function<int()> width;                 // calculate required segment width
        std::function<void(Bitmap &strip)> render;  // render the segment
    };

private:
    int const WeatherImageDx = 32;
    int const WeatherImageDy = 26;
    long _lasttime;
    float _weatherIntensity;
    std::vector<stripsegment> _stripSegments;
    int _iSegment;
    int _segmentOffset;


public:
    Application(ApplicationContext &appdata, Graphics &graphics, Environment &env, System &sys, UserInput &userinput);

    void init();
    void render();
    bool interact();
    
private:
    void drawClock(float x, float y, float diameter);
    void drawDateTimeTwoPanel(const timeinfo &now);
    void drawTimeOnePanel(const timeinfo &now);
    void drawWeather();
    void drawWeatherImage();
    void drawWindAngle();
    void drawLineFromCenter(float x, float y, float diameter, float index, float l1, float l2, float thickness, Color color);
    void drawSun(float x, float y, float dx, float dy);
    float drawCloud(float x, float y, float dx, float dy, Color line, Color fill);
    void draw2Clouds(float x, float y, float dx, float dy, Color line, Color fill);
    void drawLightning(float x, float y, float dx, float dy, Color color);
    void drawRain(float x, float y, float dx, float dy, bool heavy, Color color);
    void drawMoon(float x, float y, float dx, float dy, Color color);
    void drawStars(float x, float y, float dx, float dy);
    Color starIntensity(float phase, float when);
    void drawFog(float x, float y, float dx, float dy, Color color);
    void drawSnow(float x, float y, float dx, float dy, Color color);
};

#endif