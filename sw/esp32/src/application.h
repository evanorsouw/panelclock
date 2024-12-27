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
#include "xy.h"

class Application : public RenderBase
{   
private:
    struct stripsegment 
    {
        /**
         * @brief calculate the required width of the segment in pixels.
         * @param graphics the graphics object targetting the segment with a width equal of the panel width.
         */
        std::function<int(Graphics &view)> width;  // calculate required segment width in pixels
        /**
         * @brief lambda to render the segment using th eprovided graphics object
         * @param graphics the graphics object targetting precisely the segment dimensions.
         * @param ox subpixel x-offset in range [0,1> to render to allow smooth scrolling
         */
        std::function<void(Graphics &graphics, float ox)> render;  // render the segment
    };

private:
    long _lasttime;
    float _weatherIntensity;
    std::vector<stripsegment> _stripSegments;
    int _iSegment;
    float _segmentOffset;


public:
    Application(ApplicationContext &appdata, Environment &env, System &sys, UserInput &userinput);

    void init();
    void render(Graphics &graphics);
    bool interact();
    
private:
    void drawClock(Graphics& graphics, float x, float y, float diameter);
    void drawTimeOnePanel(Graphics& graphics, const timeinfo &now);
    void drawDateTimeTwoPanel(Graphics& graphics, const timeinfo &now);
    void drawWeatherOnePanel(Graphics& graphics);
    void drawWeatherTwoPanel(Graphics& graphics);
    void drawWindArrow(Graphics& graphics, int x, int y, int size, float angle, Color col1, Color col2);
    void drawWeatherImage(Graphics& graphics, bool onepanel);
    void drawWindAngle(Graphics& graphics);
    void drawLineFromCenter(Graphics& graphics, float x, float y, float diameter, float index, float l1, float l2, float thickness, Color color);
    void drawSun(Graphics& graphics, float x, float y, float dx, float dy);
    fxy drawCloud(Graphics& graphics, float x, float y, float dx, float dy, Color line, Color fill);
    void draw2Clouds(Graphics& graphics, float x, float y, float dx, float dy, Color line, Color fill);
    void drawLightning(Graphics& graphics, float x, float y, Color color);
    void drawRain(Graphics& graphics, float x, float y, bool heavy, Color color);
    void drawMoon(Graphics& graphics, float x, float y, float dx, float dy, Color color);
    void drawStars(Graphics& graphics, float x, float y, float dx, float dy);
    Color starIntensity(Graphics& graphics, float phase, float when);
    void drawFog(Graphics& graphics, float x, float y, float dx, float dy, Color color);
    void drawSnow(Graphics& graphics, float x, float y, float dx, float dy, Color color);

    void drawSegments(Graphics& graphics);
    stripsegment temperatureSegment();
    stripsegment windspeedSegment();
    stripsegment windangleSegment();
    stripsegment dateSegment();
    stripsegment separatorSegment();
    stripsegment colorSegment(int dx, Color color);
};

#endif