
#ifndef _APPLICATIONRUNNER_H_
#define _APPLICATIONRUNNER_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

#include "application.h"
#include "bitmap.h"
#include "configurationui.h"
#include "otaui.h"
#include "ledpanel.h"
#include "system.h"

class ApplicationRunner
{
private:
    enum class UIMode { DateTime, Config, OTA } ;
    enum class TransitionPhase { Leaving, Entering, Stable } ;

private:
    ApplicationContext &_appctx;
    LedPanel& _panel;
    Application& _appui;
    ConfigurationUI& _configui;
    OTAUI& _otaui;
    System& _system;
    Graphics &_graphics;
    QueueHandle_t _hRenderQueue;
    QueueHandle_t _hDisplayQueue;
    int _iShowScreen;
    uint64_t _totaltime;
    int _fpsInterval;
    uint64_t _totalrendertime;
    uint64_t _totaldisplaytime;
    uint64_t _lastdisplaytime;
    int _rendercount;
    UIMode _mode;
    UIMode _nextMode;
    TransitionPhase _phase;
    uint64_t _transitionStart;
    
public:
    ApplicationRunner(
        ApplicationContext& appdata, 
        LedPanel& panel, 
        Application& appui, 
        ConfigurationUI& configui, 
        OTAUI& otaui, 
        System& system,
        Graphics& graphics);

    void renderTask();
    void displayTask();
    
private:
    void startMode(UIMode mode, TransitionPhase phase);
    void startTransition(TransitionPhase phase);
    void stepGUI();
    void updateIntensity();
    void monitorRefreshRate();
};

#endif
