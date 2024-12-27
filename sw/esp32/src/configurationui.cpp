
#include <algorithm>
#include "configurationui.h"
#include "timeinfo.h"

std::vector<configchoice> ConfigurationUI::_languageChoices = { 
    configchoice("nl", "nederlands"), 
    configchoice("en", "english"), 
    configchoice("fr", "francais")  
};
std::vector<configchoice> ConfigurationUI::_dstChoices = { 
    configchoice("0", ENG_STANDARD_TIME), 
    configchoice("1", ENG_DAYLIGHT_SAVING) 
};
std::vector<configchoice> ConfigurationUI::_bootscreenChoices = { 
    configchoice("0", ENG_NO_BOOTSCREEN), 
    configchoice("1", ENG_BOOTSCREEN) 
};
std::vector<configchoice> ConfigurationUI::_secondhandChoices = { 
    configchoice("0", ENG_OPTION_SNAP), 
    configchoice("1", ENG_OPTION_SMOOTH) 
};
std::vector<configchoice> ConfigurationUI::_flipDisplayChoices = { 
    configchoice("0", ENG_USBDOWN), 
    configchoice("1", ENG_USBUP) 
};

ConfigurationUI::ConfigurationUI(ApplicationContext &appdata, Environment &env, System &sys, UserInput &userinput)
    : RenderBase(appdata, env, sys, userinput)
{
    _font = appdata.fontSettings();    

    addConfig(ENG_DST, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeyDST, _dstChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeyDST, _dstChoices); });
    addConfig(ENG_YEAR, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%04d", _system.now().year()); }, 
        [this](configline& c, bool init){ return updateYear(c, init); });
    addConfig(ENG_DATE, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%02d %s", _system.now().mday(), translate(_system.now().monthName(false)).c_str()); }, 
        [this](configline& c, bool init){ return updateDate(c, init); });
    addConfig(ENG_TIME, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%02d:%02d:%02d", _system.now().hour(), _system.now().min(), _system.now().sec()); },  
        [this](configline& c, bool init){ return updateTime(c, init); });
    addConfig(ENG_IP, 
        [this](configline& c){ generateWifiLine(c); },
        nullptr);
    addConfig(ENG_WIFI, 
        [this](configline& c){ strcpy(c.value,_system.settings().WifiSid().c_str()); }, 
        [this](configline& c, bool init){ return updateWifiSid(c, init); },
        [this](configline& c){ _system.connectWifi(); });
    addConfig(ENG_PASS, 
        [this](configline& c){ strcpy(c.value,_system.settings().WifiPassword().c_str()); }, 
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyWifiPwd); },
        [this](configline& c){ _system.connectWifi(); });
    addConfig(ENG_LANG, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeyLanguage, _languageChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeyLanguage, _languageChoices); });
    addConfig(ENG_KEY, 
        [this](configline& c){ strcpy(c.value,_system.settings().WeerliveKey().c_str()); }, 
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyWeerliveKey); },
        [this](configline& c){ _environment.update(); });
    addConfig(ENG_WEERLIVE_LOC, 
        [this](configline& c){ strcpy(c.value,_system.settings().WeerliveLocation().c_str()); }, 
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyWeerliveLocation); },
        [this](configline& c){ _environment.update(); });
    addConfig(ENG_WEERLIVE_WEATHER, 
        [this](configline& c){ generateWeatherLine(c); },
        nullptr);
    addConfig(ENG_BOOT, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeyBootscreen, _bootscreenChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeyBootscreen, _bootscreenChoices); });
    addConfig(ENG_SEC, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeySmoothSecondHand, _secondhandChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeySmoothSecondHand, _secondhandChoices); });
    addConfig(ENG_FLIP, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeyFlipDisplay, _flipDisplayChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeyFlipDisplay, _flipDisplayChoices); });
    addConfig(ENG_EXIT, 
        nullptr, 
        [this](configline&, bool){ _exitConfig = true; return false; });   
}

void ConfigurationUI::init()
{
    _configYBase = 0.0f;
    _selectionYBase = 0.0f;
    _selectedLine = 0;
    _keySetLine= 0;
    _updating = false;
    _exitConfig = false;
    _lastEditTime = _system.now();
    _iEditIndex = -1;
    _inextReaderUpdate = 0;
    selectConfig(0);

    for (auto &config: _configs)
    {
        if (config.reader)
            config.reader(config);
    }
}

void ConfigurationUI::render(Graphics &graphics)
{
    auto white = Color::white * _appctx.intensity();
    auto darkgray = Color(16,16,16) * _appctx.intensity();
    auto lime = Color::lime * _appctx.intensity();
    auto darkred = Color(19,0,0) * _appctx.intensity();

    auto labelwidth = 0.0f;
    for(auto &config : _configs)
    {
        auto info = _font->textsize(translate(config.label).c_str());
        labelwidth = std::max(labelwidth, info.dx);
    }

    auto margin = 1.0f;
    auto xvalues = labelwidth + margin * 2;

    // draw selection line background
    auto x = _updating ? xvalues : 0.0;
    auto dx = graphics.dx() - x;
    graphics.rect(x, _selectionYBase - _configYBase, dx, _font->height(), darkred);

    // draw visible configs lines
    auto view = graphics.view(xvalues, 0, graphics.dx() - xvalues, graphics.dy(), true);
    for (auto i=0; i<_configs.size(); ++i)
    {
        auto &config = _configs[i];
        auto yt = i * _font->height() - _configYBase;
        auto yb = yt + _font->height();
        auto color = config.updater ? white : lime;

        if (yb < 0 || yt >= view.dy())
            continue;

        auto y = yt + _font->height() + _font->descend();

        // draw label
        graphics.text(_font, margin, y, translate(config.label).c_str(), white);

        // draw value
        if (i != _selectedLine || _iEditIndex < 0)
        {            
            auto x = config._xScrollOffset;
            x = view.text(_font, x, y, config.value, color);
            switch (config._xScrollState)
            {
            case ScrollState::Begin:
                config._xScrollOffset = 0;
                if (timeout(config._scrolldelay, 1000))
                {
                    starttimer(config._scrolldelay);
                    config._xScrollState = ScrollState::Scrolling;
                }
                break;
            case ScrollState::Scrolling:
                if (x < view.dx())
                {
                    config._xScrollState = ScrollState::End;
                    starttimer(config._scrolldelay);
                }
                else
                {
                    config._xScrollOffset -= std::min(1.0f, (float)elapsed(config._scrolldelay/5));
                }
                break;
            case ScrollState::End:
                if (timeout(config._scrolldelay, 1500))
                {
                    config._xScrollState = ScrollState::Begin;
                    starttimer(config._scrolldelay);
                }
                break;
            }
        }
    }

    // draw textual editing interface (vertical character column)
    if (_iEditIndex >= 0)
    {
        auto neditchars = strlen(_editChars);
        auto &config =  _configs[_selectedLine];

        auto x = xvalues + config._xScrollOffset;
        auto xroll = x;

        // draw text and empty vertical character-roll
        auto yt = _selectedLine * _font->height() - _configYBase;
        auto yb = yt + _font->height();
        auto y = yt + _font->height() + _font->descend() - 2;
        auto targetXScrollOffset = config._xScrollOffset;
        for (auto i=0; i < sizeof(config.value); ++i)
        {
            auto c = config.value[i];
            if (i == _iEditIndex)
            {
                xroll = x;
                x += _rollXOffset;
                graphics.rect(x, 0, _font->sizex(), 64, darkgray);
                graphics.line(x, 0, x, 64, 0.5, white);
                graphics.line(x + _font->sizex() - 1, 0, x + _font->sizex() - 1, 64, 0.5f, white);
                x += _font->sizex() - _rollXOffset;
                targetXScrollOffset = config._xScrollOffset + std::min(0.0f, graphics.dx() - x);
            }
            else
            {
                x = graphics.text(_font, x, y,  c, white);
            }            
            if (c == 0 || c == AcceptChar)
                break;
        }        

        auto croll = config.value[_iEditIndex];
        auto iroll = rollIndex(croll);
        yb += _rollYOffset;
        while (yb >= 0)
        {
            yb -= _font->height();
            iroll = (iroll == 0) ? neditchars - 1 : iroll -1;
        }
        x = xroll + _rollXOffset;
        yt = yb - _font->height();
        while (yt < 64)
        {
            auto y = yt;
            auto dx = _font->sizex();
            auto dy = _font->sizey();
            if (_editChars[iroll] == AcceptChar)
            {
                graphics.triangle(x+1, y+3, x+3, y+3, x+dx/3, y+dy-1, lime);
                graphics.triangle(x+dx-3, y+1, x+dx, y+1, x+dx/3, y+dy-1, lime);
            }
            else
            {
                auto w = _font->charsize(_editChars[iroll]).dx;
                y = yt + _font->height() + _font->descend() - 2;
                auto xc = x + (_font->sizex() - w) / 2;
                graphics.text(_font, xc, y, _editChars[iroll], white);
            }
            yt += _font->height();
            iroll = (iroll + 1) % neditchars;
        }
        graduallyUpdateVariable(_rollYOffset, 0, 2);
        graduallyUpdateVariable(_rollXOffset, 0, 2);
        graduallyUpdateVariable(config._xScrollOffset, targetXScrollOffset, 2);
    }
    graduallyUpdateVariable(_configYBase, _selectedLine * _font->height() - 64 + _font->height(), _selectedLine * _font->height(), 0.5);
    graduallyUpdateVariable(_selectionYBase, _selectedLine * _font->height(), 0.5);
}

bool ConfigurationUI::interact()
{
    {
        auto &config = _configs[_inextReaderUpdate];
        if (config.reader && (!_updating || _inextReaderUpdate != _selectedLine))
        {
            config.reader(config);
        }
        _inextReaderUpdate = (_inextReaderUpdate + 1) % _configs.size();
    }

    auto &config = _configs[_selectedLine];
    if (_updating)
    {
        _updating = config.updater(config, false);
        if (!_updating && config.onexitaction)
        {
            config.onexitaction(config);
        }
        _lastEditTime = _system.now();
        _keySetLine = _selectedLine;
    }
    else
    {
        _iEditIndex = -1;

        auto key = getKey();
        if (key == UserInput::KEY_SET)
        {
            initEditRoll(config);
            config.updater(config, true);
            _updating = true;
        }
        else
        {
            repeatUpdateOnKey(UserInput::KEY_DOWN, key, [&](float delta){ _keySetLine += delta; });
            repeatUpdateOnKey(UserInput::KEY_UP, key, [&](float delta){ _keySetLine -= delta; });
        }
        _keySetLine = std::max(0.0f, std::min(_keySetLine, _configs.size() - 1.0f));
        if ((int)_keySetLine != _selectedLine)
        {
            selectConfig((int)_keySetLine);
            _keySetLine = _selectedLine;
        }
    }

    auto exit = _exitConfig || isTimeout();
    if (exit)
    {
        _system.settings().saveSettings();
        _system.connectWifi();
    }
    return exit;
}

void ConfigurationUI::addConfig(const char *label,
    std::function<void(configline &)> reader,
    std::function<bool(configline &, bool)> updater,
    std::function<void(configline &)> onexit)
{
    _configs.push_back(configline());
    auto &config = _configs.back();

    config.label = label;
    config.value[0] = 0;
    config.reader = reader;
    config.updater = updater;
    config.onexitaction = onexit;
}

void ConfigurationUI::selectConfig(int i)
{
    if (i < _selectedLine)
    {
        while (!_configs[i].updater && i > 0 && i < _configs.size())
        {
            i--;
        }
    }
    else
    {
        while(!_configs[i].updater && i >= 0 && (i + 1 < _configs.size()))
        {
            i++;
        }
    }
    if (i>=0 && i < _configs.size() && _configs[i].updater)
    {
        _selectedLine = i;
        printf("selectedline=%d\n", _selectedLine);
    }        
}

bool ConfigurationUI::updateYear(configline& config, bool init)
{
    if (init)
    {
        config.setpoint = _system.now().year();
    }

    auto editing = !isEditTimeout();
    if (editing)
    {
        auto key = getKey();
        editing = key != UserInput::KEY_SET;
        if (editing)
        {
            repeatUpdateOnKey(UserInput::KEY_DOWN, key, [&](float delta){ config.setpoint += delta; });
            repeatUpdateOnKey(UserInput::KEY_UP, key, [&](float delta){ config.setpoint -= delta; });
        }
        config.setpoint = std::max(1970.0f, std::min(2099.0f, config.setpoint));
        auto now = _system.now();
        now.setDate((int)config.setpoint, now.mon(), now.mday());
        snprintf(config.value, sizeof(config.value), "%04d", now.year());

        if (!editing)
        {
            _system.now(now);
        }
    }
    return editing;
}

bool ConfigurationUI::updateDate(configline& config, bool init)
{
    if (init)
    {
        config.setpoint = _system.now().yday();
    }

    auto editing = !isEditTimeout();
    if (editing)
    {
        auto key = getKey();
        editing = key != UserInput::KEY_SET;
        if (editing)
        {
            repeatUpdateOnKey(UserInput::KEY_DOWN, key, [&](float delta){ config.setpoint += delta; });
            repeatUpdateOnKey(UserInput::KEY_UP, key, [&](float delta){ config.setpoint -= delta; });
        }
        auto now = _system.now();
        auto hasleap = (now.year() % 4 == 0) && (now.year() % 400 == 0 || now.year() % 100 != 0);
        auto maxdays = hasleap ? 366 : 365;
        if (config.setpoint < 0)
            config.setpoint += maxdays;
        else if (config.setpoint >= maxdays)
            config.setpoint -= maxdays;

        auto yday = (int)config.setpoint + 1;
        auto month = 0;
        auto mday = yday;
        while (mday > timeinfo::daysInMonth(month, hasleap))
        {
            mday -= timeinfo::daysInMonth(month, hasleap);
            month++;
        }

        now.setDate(now.year(), month, mday);
        snprintf(config.value, sizeof(config.value),  "%02d %s", now.mday(), translate(now.monthName(false)).c_str());
        
        if (!editing)
        {
            _system.now(now);
        }
    }
    return editing;
}

bool ConfigurationUI::updateTime(configline& config, bool init)
{
    if (init)
    {
        auto now = _system.now();
        config.setpoint = now.hour() * 60 + now.min();
    }

    auto editing = !isEditTimeout();
    if (editing)
    {
        auto key = getKey();
        editing = key != UserInput::KEY_SET;
        if (editing)       
        {
            repeatUpdateOnKey(UserInput::KEY_DOWN, key, [&](float delta){ config.setpoint += delta; });
            repeatUpdateOnKey(UserInput::KEY_UP, key, [&](float delta){ config.setpoint -= delta; });
        }
        auto max = 24 * 60;
        if (config.setpoint < 0)
            config.setpoint += max;
        else if (config.setpoint >= max)
            config.setpoint -= max;
        
        auto now = _system.now();
        now.setTime((int)config.setpoint / 60, (int)config.setpoint % 60, 0);
        snprintf(config.value, sizeof(config.value), "%02d:%02d:%02d", now.hour(), now.min(), now.sec());

        if (!editing)
        {
            _system.now(now);
        }
    }
    return editing;
}

bool ConfigurationUI::updateWifiSid(configline& config, bool init)
{
    if (init)
    {
        _system.scanAPs();
    }

    auto editing = !isEditTimeout();
    if (editing)
    {
        auto naps = _system.nAPs();

        auto key = getKey();
        auto idx = (int)config.setpoint;
        auto changed = false;
        switch (key)
        {
            case UserInput::KEY_SET:
                editing = false;
                break;
            case UserInput::KEY_UP:
                idx++;
                changed = true;
                break;
            case UserInput::KEY_DOWN:
                idx--;
                changed = true;
                break;
        }

        if (changed && naps > 0)
        {            
            idx = (idx + naps) % naps;
            config.setpoint = idx;
            snprintf(config.value, sizeof(config.value), "%s", _system.APSID(idx));
        }
        if (!editing)
        {
            _system.settings().WifiSid(config.value);
        }
    }        
    return editing;
}

void ConfigurationUI::generateSettingLine(configline& config, const char *settingKey, const std::vector<configchoice> choices) const
{
    auto store = _system.settings().get(settingKey)->asstring().c_str();
    auto it = std::find_if(choices.begin(), choices.end(), [&](const configchoice &c) { return !strcmp(c.store, store); } );
    if (it == choices.end())
        it = choices.begin();
    snprintf(config.value, sizeof(config.value), "%s", translate(it->english).c_str());
}

bool ConfigurationUI::updateSettingFreeText(configline& config, bool init, const char *settingkey)
{
    if (init)
    {
        _iEditIndex = 0;
        memset(config.value + strlen(config.value), 0, sizeof(config.value) - strlen(config.value));
        config.setpoint = rollIndex(config.value[_iEditIndex]);
    }

    auto editing = !isEditTimeout();
    if (editing)
    {
        auto key = getKey();
        auto pval = config.value;
        if (key == UserInput::KEY_SET)
        {
            _rollXOffset = -_font->charsize(config.value[_iEditIndex]).dx;
            if (pval[_iEditIndex] == AcceptChar)
            {
                pval[_iEditIndex] = 0;
                editing = false;
            }
            else
            {
                _iEditIndex = std::min((int)sizeof(config.value) - 1, _iEditIndex + 1);
                _rollYOffset = 0.0f;
                config.setpoint = rollIndex(pval[_iEditIndex]);
            }
        }
        if (editing)
        {
            auto setpoint = config.setpoint;
            repeatUpdateOnKey(UserInput::KEY_DOWN, key, [&](float delta){ setpoint += delta; });
            repeatUpdateOnKey(UserInput::KEY_UP, key, [&](float delta){ setpoint -= delta; });

            if (setpoint != config.setpoint)
            {
                auto neditchars = strlen(_editChars);
                auto direction = (setpoint < config.setpoint) ? -1 : 1;
                if (setpoint < 0)
                    setpoint += neditchars;
                else if (setpoint >= neditchars)
                    setpoint -= neditchars;
                config.setpoint = setpoint;
                _rollYOffset = direction * _font->height();
            }
            auto rollIndex = (int)config.setpoint;
            pval[_iEditIndex] = _editChars[rollIndex];
        }
        else
        {
            _iEditIndex = -1;
            _system.settings().get(settingkey)->set(config.value);
        }
    }
    return editing;
}

bool ConfigurationUI::updateSettingChoices(configline& config, bool init, const char *settingKey, const std::vector<configchoice> &choices)
{
    if (init)
    {
        auto store = _system.settings().get(settingKey)->asstring().c_str();
        for (int i=0; i<choices.size(); ++i)
        {
            if (!strcmp(choices[i].store, store))
                config.setpoint = i;
        }
    }

    auto editing = !isEditTimeout();
    if (editing)
    {
        auto key = getKey();
        auto idx = (int)config.setpoint;
        switch (key)
        {
            case UserInput::KEY_SET:
                _system.settings().get(settingKey)->set(choices[idx].store);
                editing = false;
                break;
            case UserInput::KEY_UP:
                idx++;
                break;
            case UserInput::KEY_DOWN:
                idx--;
                break;
        }

        idx = (idx + choices.size()) % choices.size();
        config.setpoint = idx;
        snprintf(config.value, sizeof(config.value), "%s", translate(choices[idx].english).c_str());
    }
    return editing;
}

int ConfigurationUI::getKey() 
{ 
    auto keypress = _userinput.getKey(); 
    if (keypress.key != 0)
    {
        _lastEditTime = _system.now();
    }
    return keypress.key;
}

KeyPress ConfigurationUI::getKeyPress() 
{ 
    auto keypress = _userinput.getKey(); 
    if (keypress.key != 0)
    {
        _lastEditTime = _system.now();
    }
    return keypress;
}

void ConfigurationUI::repeatUpdateOnKey(int activateKey, int keyPressed, std::function<void(float)> handler)
{
    auto period = std::max(0, (int)_userinput.howLongIsKeyDown(activateKey) - 200) / 3000.0f;
    period = std::min(0.5f, period);
    auto delta = (keyPressed == activateKey) ? 1.0f : 0.0f;
    delta += pow10(period) - 1.0f;
    handler(delta);
}

bool ConfigurationUI::isEditTimeout()
{
    return _userinput.hasKeyDown(UserInput::KEY_SET, 2000);
}

bool ConfigurationUI::isTimeout()
{
    auto elapsed = _system.now().msticks() - _lastEditTime.msticks();
    if (elapsed < 0)
    {
        _lastEditTime = _system.now();
    }
    return elapsed > 300*1000;
}

void ConfigurationUI::generateWifiLine(configline& config)
{
    if (_system.wifiConnected()) 
    {
        snprintf(config.value, sizeof(config.value), "%s", _system.IPAddress()); 
        return;
    }

    auto step = (_system.now().msticks() / 300) % 4;
    char dots[5] = "    ";
    dots[step] = '.';
    if (_system.wifiScanning()) 
    {
        snprintf(config.value, sizeof(config.value), "%s%s(%d)", translate(ENG_SCANNING).c_str(), dots, _system.nAPs());
    }
    else if (_system.wifiConnecting()) 
    {
        snprintf(config.value, sizeof(config.value), "%s%s", translate(ENG_CONNECTING).c_str(), dots);
    }
    else
    {
        strcpy(config.value, "");
    }
}

void ConfigurationUI::generateWeatherLine(configline& config)
{
    if (!_system.wifiConnected())
    {
        snprintf(config.value, sizeof(config.value), "%s", translate(ENG_NO_INTERNET).c_str());
    }
    else if (_environment.isupdating())
    {
        auto step = (_system.now().msticks() / 400) % 4;
        char dots[5] = "    ";
        dots[step] = '.';
        snprintf(config.value, sizeof(config.value), "%s%s", translate(ENG_UPDATING).c_str(), dots);
    }
    else if (_environment.valid())
    {
        auto location = translate("somewhere").c_str();
        if (_environment.location().isValid() && _environment.location().value() != "")
        {
            location = _environment.location().value().c_str();
        }
        snprintf(config.value, sizeof(config.value), "%s, %.1f°", 
            location, _environment.temperature().value());
    }
    else
    {
        snprintf(config.value, sizeof(config.value), "%s", _environment.invalidReason().c_str()); 
    }
}

void ConfigurationUI::initEditRoll(configline  &config)
{
    _rollXOffset = 0;
    _rollYOffset = 0;
    config._xScrollOffset = 0;
}

int ConfigurationUI::rollIndex(char c)
{
    if (c == 0)
        return 0;
    auto proll = strchr(_editChars, c);
    if (!proll)
        return 0;
    return proll - _editChars;   
}
