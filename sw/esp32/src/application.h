#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <limits>

#include "animation.h"
#include "bitmap.h"
#include "ds3231.h"
#include "environment.h"
#include "graphics.h"
#include "ledpanel.h"
#include "spline.h"
#include "system.h"

class Application
{
private:
    Graphics &_graphics;
    LedPanel &_panel;
    Environment &_environment;
    System &_system;
    QueueHandle_t _hRenderQueue;
    QueueHandle_t _hDisplayQueue;
    Font *_fonttimeLarge;
    Font *_fonttimeSmall;
    Font *_fontdate;
    Font *_fontweatherL;
    Font *_fontweatherS;
    Font *_fontIcons8;
    Font *_fontIcons9;
    int _refreshCount;
    long _refreshCountStart;
    int _iShowScreen;
    long _msSinceMidnight;
    int _bootAnimationPhase;
    int64_t _bootStart;
    std::vector<Animation> _bootAnimations;

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
    void drawWindAngle(Bitmap &screen);
    void drawCenterLine(Bitmap &screen, float x, float y, float diameter, float index, float l1, float l2, float thickness, Color color);
    void sendScreen();
    void showScreenOnPanel();
    void monitorRefreshRate(long ms);
    float phase(float ms, bool wave);
    void drawSun(Bitmap &screen, float x, float y, float dx, float dy);
    long drawtime() { return _msSinceMidnight; }    

    bool animateBackground(Bitmap &screen, float when);
    bool animatePackageBox(Bitmap &screen, float when);
};

#endif