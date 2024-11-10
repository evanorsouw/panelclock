
#ifndef _CONFIGURATIONUI_H_
#define _CONFIGURATIONUI_H_

#include <vector>
#include <functional>

#include "environment.h"
#include "font.h"
#include "graphics.h"
#include "system.h"
#include "timeinfo.h"
#include "userinput.h"

struct configline 
{
    configline () 
    {
        label = nullptr;
        value[0] = 0;
    }
    const char *label;
    char value[40];    
    std::function<void(configline &, bool)> initter;
    std::function<void(configline &)> reader;
    std::function<bool(configline &, bool)> updater;
    timeinfo start;
    float setpoint;
};

class ConfigurationUI
{
private:
    Graphics &_graphics;
    Environment &_env;
    System &_sys;
    UserInput &_userinput;
    Font *_font;
    int _selectedLine;
    float _labelwidth;
    float _configYBase;
    float _selectionYBase;
    std::vector<configline> _configs;
    bool _updating;
    bool _exitConfig;
    timeinfo _lastEditTime;
    int _iCharacterRoll;
    int _iEditIndex;

public:
    ConfigurationUI(Graphics &graphics, Environment &env, System &sys, UserInput &userinput, Font *font);

    bool render(Bitmap &screen);
    void startConfigurationSession();
    void endConfigurationSession();

private:
    void addConfig(const char *label,
        std::function<void(configline &, bool)> initter,
        std::function<void(configline &)> reader,
        std::function<bool(configline &, bool)> updater);
    void selectConfig(int i);
    void drawConfigLines(Bitmap &screen);
    bool updateDST(configline &config, bool init);
    bool updateWifiSid(configline &config, bool init);
    bool updateWifiPassword(configline &config, bool init);
    bool updateYear(configline &config, bool init);
    bool updateDate(configline &config, bool init);
    bool updateTime(configline &config, bool init);
    void repeatUpdateOnKey(int activateKey, int pressedKey, std::function<void(float)> handler);
    int getKey();
    bool isEditTimeout();
    bool isTimeout();
    void generateDSTLine(configline &config);
    void generateWifiLine(configline &config);
    void runInitter(bool init);
};

#endif
