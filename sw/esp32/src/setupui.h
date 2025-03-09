
#ifndef _SETUPUI_H_
#define _SETUPUI_H_

#include <string>
#include <vector>

#include "ledpanel.h"
#include "renderbase.h"
#include "version.h"

class SetupUI : public RenderBase
{
private:
    LedPanel &_ledPanel;
    uint64_t _resettimer;
    bool _resetStarted;

public:
    SetupUI(ApplicationContext &appdata, IEnvironment &env, System &sys, UserInput &userinput, LedPanel &panel)
        : RenderBase(appdata, env, sys, userinput)
        , _ledPanel(panel)
    {}

    void init() override;
    void render(Graphics &graphics) override;
    int interact() override;

private:
    void draw1Panel(Graphics &graphics);
    void draw2PanelLandscape(Graphics &graphics);
    void draw2PanelPortrait(Graphics &graphics);
    void drawBorder(Graphics &graphics, float d);
};

#endif
