
#ifndef _APPLICATIONRUNNER_H_
#define _APPLICATIONRUNNER_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

#include "application.h"
#include "bitmap.h"
#include "bootanimations.h"
#include "configurationui.h"
#include "ledpanel.h"
#include "system.h"

class ApplicationRunner
{
private:
    enum class UIMode { Boot, DateTime, Config } ;
    enum class TransitionPhase { Leaving, Entering, Stable } ;

private:
    ApplicationContext &_appctx;
    LedPanel& _panel;
    BootAnimations& _bootui;
    Application& _appui;
    ConfigurationUI& _configui;
    System& _system;
    Graphics &_graphics;
    QueueHandle_t _hRenderQueue;
    QueueHandle_t _hDisplayQueue;
    int _iShowScreen;
    uint64_t _totaltime;
    uint64_t _totalrendertime;
    uint64_t _totaldisplaytime;
    int _rendercount;
    UIMode _mode;
    UIMode _nextMode;
    TransitionPhase _phase;
    uint64_t _transitionStart;
    
public:
    ApplicationRunner(
        ApplicationContext& appdata, 
        LedPanel& panel, 
        BootAnimations& bootui, 
        Application& appui, 
        ConfigurationUI& configui, 
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
