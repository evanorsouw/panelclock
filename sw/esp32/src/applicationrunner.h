
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
#include "setupui.h"
#include "system.h"

class ApplicationRunner
{
private:
    enum class UIMode { DateTime, Config, OTA, Setup } ;
    enum class TransitionPhase { Leaving, Entering, Stable } ;

private:
    ApplicationContext &_appctx;
    LedPanel& _panel;
    Application& _appui;
    ConfigurationUI& _configui;
    OTAUI& _otaui;
    SetupUI& _setupui;
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
    bool _automaticSoftwareUpdate;
    
public:
    ApplicationRunner(
        ApplicationContext& appdata, 
        LedPanel& panel, 
        Application& appui, 
        ConfigurationUI& configui, 
        OTAUI& otaui, 
        SetupUI& setupui, 
        System& system,
        Graphics& graphics);

    void render();
    void display(); 
    void newVersionAvailable() { _automaticSoftwareUpdate = true; }

private:
    void startMode(UIMode mode, TransitionPhase phase);
    void startTransition(TransitionPhase phase);
    void render(Graphics &graphics);
    void updateIntensity();
    void monitorRefreshRate();
};

#endif
