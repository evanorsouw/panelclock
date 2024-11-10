
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

enum class CallReason { Running, Init, DeInit };

struct configline 
{
    configline () 
    {
        label = nullptr;
        value[0] = 0;
    }
    const char *label;
    char value[40];    
    std::function<void(configline &)> reader;
    std::function<bool(configline &, bool)> updater;
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
    int _iEditIndex;
    static constexpr const char *_editChars = "\x01\x02""aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ_ !?$@#%^&*()-+={}[]<>:;\"`~',./|\\";
    static const char AcceptChar = 0x01;
    static const char DeleteChar = 0x02;

public:
    ConfigurationUI(Graphics &graphics, Environment &env, System &sys, UserInput &userinput, Font *font);

    bool render(Bitmap &screen);
    void startConfigurationSession();
    void endConfigurationSession();

private:
    /// @brief add a configurable item.
    /// @param label the text that appears in front of the confguration item to indicate its purpose.
    /// @param reader optional the lambda that is called with each refresh of the screen to update 
    /// the config's current value. This is not called when the configuration item is in edit-mode.
    /// @param updater the optional lambda that is called when the config is in edit-mode. The init
    /// flags is true first call in a edit series abnd may be used for initialization.
    /// When updater is nullptr, the item becomes readonly.
    void addConfig(const char *label,
        std::function<void(configline &cfg)> reader,
        std::function<bool(configline &cfg, bool init)> updater);
        
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
    KeyPress getKeyPress();
    bool isEditTimeout();
    bool isTimeout();
    const char *translate(const char *txt) const { return _sys.translate(txt); }
    void generateDSTLine(configline &config, bool dst);
    void generateWifiLine(configline &config);
    int rollIndex(char c);
};

#endif
