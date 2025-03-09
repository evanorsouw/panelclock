
#ifndef _CONFIGURATIONUI_H_
#define _CONFIGURATIONUI_H_

#include <vector>
#include <functional>
#include <esp_timer.h>

#include "environmentselector.h"
#include "font.h"
#include "graphics.h"
#include "renderbase.h"
#include "system.h"
#include "timeinfo.h"
#include "userinput.h"

enum class ScrollState { Begin, Scrolling, End };

struct configline 
{
    configline () 
    {
        label = nullptr;
        value[0] = 0;
        xValueScrollState = ScrollState::Begin;
        xValueScrollOffset = 0;
        xValueScrollDelay = 0;
        xLabelScrollState = ScrollState::Begin;
        xLabelScrollOffset = 0;
        xLabelScrollDelay = 0;
    }
    const char *label;
    char value[50];
    std::function<void(configline&)> reader;
    std::function<bool(configline&, bool)> updater;
    std::function<bool(configline&)> visible;
    std::function<void(configline&)> onexitaction;
    float setpoint;
    float xLabelScrollOffset;
    ScrollState xLabelScrollState;
    uint64_t xLabelScrollDelay;
    float xValueScrollOffset;
    ScrollState xValueScrollState;
    uint64_t xValueScrollDelay;
};

struct configchoice
{
    const char *store;
    const char *english;
};

class ConfigurationUI : public RenderBase
{
private:
    static constexpr const char *_editChars = "\x01""0123456789aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ_ !?$@#%^&*()-+={}[]<>:;\"`~',./|\\";
    static const char AcceptChar = 0x01;
    static std::vector<configchoice> _languageChoices;
    static std::vector<configchoice> _tzChoices;
    static std::vector<configchoice> _bootscreenChoices;
    static std::vector<configchoice> _secondhandChoices;
    static std::vector<configchoice> _orientationChoices;
    static std::vector<configchoice> _timeModeChoices;
    static std::vector<configchoice> _weatherChoices;
    static std::vector<configchoice> _flipKeyChoices;

    Font *_font;
    float _margin;
    float _labelwidth;
    float _keySetLine;
    int _selectedLine;
    float _configYBase;
    float _selectionYBase;
    std::vector<configline> _configs;
    int _inextReaderUpdate;
    bool _updating;
    int _exitCode;
    timeinfo _lastEditTime;
    int _iEditIndex;
    float _rollYOffset;
    float _rollXOffset;

public:
    ConfigurationUI(ApplicationContext &appdata, EnvironmentSelector &env, System &sys, UserInput &userinput);

    void render(Graphics& graphics);
    int interact();
    void init();

private:
    /// @brief add a configurable item.
    /// @param label the text that appears in front of the confguration item to indicate its purpose.
    /// @param reader optional the lambda that is called with each refresh of the screen to update 
    /// the config's current value. This is not called when the configuration item is in edit-mode.
    /// @param updater the optional lambda that is called when the config is in edit-mode. The init
    /// flags is true first call in a edit series abnd may be used for initialization.
    /// When updater is nullptr, the item becomes readonly.
    /// @param visible optional lambda that is used to dynamically (e.g. based on other settings)
    /// determine if this setting is visible or not.
    /// When visible is nullptr, the setting is visible.
    /// @param onexitaction the optional lambda called when updating the config ends. can be used
    /// to trigger behaviour to take the new settings into account.
    void addConfig(const char *label,
        std::function<void(configline& cfg)> reader,
        std::function<bool(configline& cfg, bool init)> updater = nullptr,
        std::function<bool(configline& cfg)> visible = nullptr,
        std::function<void(configline& cfg)> onexitaction = nullptr);
    configline& getConfig(int i);
    int nConfigs();

    void calculateLabelWidth();
    void updateScrollState(ScrollState &scrollstate, float &scrolloffset, uint64_t &scrolldelay, bool endfits);
    void selectConfig(int i);
    bool updateWifiSid(configline &config, bool init);
    void generateSettingLine(configline &line, const char *settingKey, const std::vector<configchoice> choices) const;
    bool updateSettingFreeText(configline &config, bool init, const char *settingKey);
    bool updateSettingInteger(configline &config, bool init, const char *settingKey, int min, int max);
    bool updateSettingChoices(configline &config, bool init, const char *settingKey, const std::vector<configchoice> &choices);
    bool updateYear(configline &config, bool init);
    bool updateDate(configline &config, bool init);
    bool updateTime(configline &config, bool init);
    void repeatUpdateOnKey(int activateKey, int pressedKey, std::function<void(float)> handler);
    int getKey();
    KeyPress getKeyPress();
    bool isEditCancelled();
    bool isTimeout();
    const std::string &translate(const std::string &txt) const { return _system.translate(txt); }
    void generateWifiLine(configline &config);
    void generateWeatherLine(configline &config);
    void initEditRoll(configline  &config);
    int rollIndex(char c);
};

#endif
