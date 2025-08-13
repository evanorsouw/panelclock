#include <algorithm>

#include "configurationui.h"
#include "environment_weerlive.h"
#include "environment_openweather.h"
#include "environmentselector.h"
#include "graphics.h"
#include "timeinfo.h"
#include "version.h"

std::vector<configchoice> ConfigurationUI::_languageChoices = { 
    configchoice("nl", "nederlands"), 
    configchoice("en", "english"), 
    configchoice("fr", "francais")  
};
// timezones retrieved from
// https://raw.githubusercontent.com/nayarsystems/posix_tz_db/refs/heads/master/zones.csv
std::vector<configchoice> ConfigurationUI::_tzChoices = { 
   configchoice("GMT0BST,M3.5.0/1, M10.5.0", "London"),
   configchoice("WET0WEST,M3.5.0/1, M10.5.0", "Lisbon"),
   configchoice("IST-1GMT0,M10.5.0, M3.5.0/1", "Dublin"),
   configchoice("CET-1CEST,M3.5.0, M10.5.0/3", "Amsterdam"),
   configchoice("EET-2EEST,M3.5.0/3, M10.5.0/4", "Athens"),   
   configchoice(TZ_CUSTOM, "Custom"),
};
std::vector<configchoice> ConfigurationUI::_bootscreenChoices = { 
    configchoice("0", ENG_NO_BOOTSCREEN), 
    configchoice("1", ENG_BOOTSCREEN) 
};
std::vector<configchoice> ConfigurationUI::_secondhandChoices = { 
    configchoice("0", ENG_OPTION_SNAP), 
    configchoice("1", ENG_OPTION_SMOOTH) 
};
std::vector<configchoice> ConfigurationUI::_orientationChoices = { 
    configchoice("0", ENG_ROTATE_0), 
    configchoice("1", ENG_ROTATE_90),
    configchoice("2", ENG_ROTATE_180), 
    configchoice("3", ENG_ROTATE_270) 
};
std::vector<configchoice> ConfigurationUI::_timeModeChoices = { 
    configchoice("0", ENG_AUTOMATIC), 
    configchoice("1", ENG_MANUAL) 
};
std::vector<configchoice> ConfigurationUI::_flipKeyChoices = { 
    configchoice("0", ENG_KEYS_NORMAL), 
    configchoice("1", ENG_KEYS_REVERSED) 
};
std::vector<swupdatechoice> ConfigurationUI::_swUpdateChoices = { 
    swupdatechoice(0, ENG_UPDATE_MANUAL), 
    swupdatechoice(1, ENG_UPDATE_MINUTELY), 
    swupdatechoice(60, ENG_UPDATE_HOURLY), 
    swupdatechoice(24*60, ENG_UPDATE_DAILY), 
    swupdatechoice(7*24*60, ENG_UPDATE_WEEKLY), 
};
std::vector<configchoice> ConfigurationUI::_weatherChoices;

ConfigurationUI::ConfigurationUI(ApplicationContext &appdata, EnvironmentSelector &env, System &sys, UserInput &userinput)
    : RenderBase(appdata, env, sys, userinput)
{
    _font = appdata.fontSettings();    
    _margin = 1.0f;
    
    addConfig(ENG_VERSION, 
        [this](configline& c){ strcpy(c.value, Version::application().astxt()); },
        [this](configline& c, bool init){ if (!init) { _exitCode = 2; } return false; });
    addConfig(ENG_TZ, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeyTZ, _tzChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeyTZ, _tzChoices); });
    addConfig(ENG_TZ_CUSTOM, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%s", _system.settings().TZCustom().c_str()); },  
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyTZCustom); },
        [this](configline& c){ return _system.settings().TZ() == TZ_CUSTOM; });        
    addConfig(ENG_TIME_MODE, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeyTimeMode, _timeModeChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeyTimeMode, _timeModeChoices); });
    addConfig(ENG_TIME, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%02d:%02d:%02d", _system.now().hour(), _system.now().min(), _system.now().sec()); },  
        nullptr,
        [this](configline& c){ return _system.settings().TimeMode() == 0; });        
    addConfig(ENG_TIME, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%02d:%02d:%02d", _system.now().hour(), _system.now().min(), _system.now().sec()); },  
        [this](configline& c, bool init){ return updateTime(c, init); },
        [this](configline& c){ return _system.settings().TimeMode() == 1; });        
    addConfig(ENG_DATE, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%02d %s", _system.now().mday(), translate(_system.now().monthName(false)).c_str()); }, 
        [this](configline& c, bool init){ return updateDate(c, init); },
        [this](configline& c){ return _system.settings().TimeMode() == 1; });        
    addConfig(ENG_DATE, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%02d %s", _system.now().mday(), translate(_system.now().monthName(false)).c_str()); }, 
        nullptr,
        [this](configline& c){ return _system.settings().TimeMode() == 0; });        
    addConfig(ENG_YEAR, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%04d", _system.now().year()); }, 
        [this](configline& c, bool init){ return updateYear(c, init); },
        [this](configline& c){ return _system.settings().TimeMode() == 1; });        
    addConfig(ENG_YEAR, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%04d", _system.now().year()); }, 
        nullptr,
        [this](configline& c){ return _system.settings().TimeMode() == 0; });        
    addConfig(ENG_NTP_SERVER, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%s", _system.settings().NTPServer().c_str()); },  
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyNTPServer); },
        [this](configline& c){ return _system.settings().TimeMode() == 0; });        
    addConfig(ENG_NTP_INTERVAL, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%d:%02d", 
            _system.settings().NTPInterval()/60, _system.settings().NTPInterval()%60); },  
        [this](configline& c, bool init){ return updateSettingInteger(c, init, AppSettings::KeyNTPInterval, 1, 24*60); },
        [this](configline& c){ return _system.settings().TimeMode() == 0; });        
    addConfig(ENG_IP, 
        [this](configline& c){ generateWifiLine(c); });
    addConfig(ENG_WIFI, 
        [this](configline& c){ strcpy(c.value,_system.settings().WifiSid().c_str()); }, 
        [this](configline& c, bool init){ return updateWifiSid(c, init); },
        nullptr,
        [this](configline& c){ _system.connectWifi(); });
    addConfig(ENG_PASS, 
        [this](configline& c){ strcpy(c.value,_system.settings().WifiPassword().c_str()); }, 
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyWifiPwd); },
        nullptr,
        [this](configline& c){ _system.connectWifi(); });
    addConfig(ENG_LANG, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeyLanguage, _languageChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeyLanguage, _languageChoices); });
    addConfig(ENG_WEATHER_SOURCE, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeyWeatherSource, _weatherChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeyWeatherSource, _weatherChoices); },
        nullptr,
        [this](configline& c){ _environment.triggerUpdate(); });
    addConfig(ENG_WEATHER_KEY, 
        [this](configline& c){ strcpy(c.value,_system.settings().WeerliveKey().c_str()); }, 
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyWeerliveKey); },
        [this](configline& c){ return _system.settings().WeatherSource() == EnvironmentWeerlive::name(); },      
        [this](configline& c){ _environment.triggerUpdate(); });
    addConfig(ENG_WEATHER_LOC, 
        [this](configline& c){ strcpy(c.value,_system.settings().WeerliveLocation().c_str()); }, 
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyWeerliveLocation); },
        [this](configline& c){ return _system.settings().WeatherSource() == EnvironmentWeerlive::name(); }, 
        [this](configline& c){ _environment.triggerUpdate(); });
    addConfig(ENG_WEATHER_KEY, 
        [this](configline& c){ strcpy(c.value,_system.settings().OpenWeatherKey().c_str()); }, 
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyOpenWeatherKey); },
        [this](configline& c){ return _system.settings().WeatherSource() == EnvironmentOpenWeather::name(); },      
        [this](configline& c){ _environment.triggerUpdate(); });
    addConfig(ENG_WEATHER_LOC, 
        [this](configline& c){ strcpy(c.value,_system.settings().OpenWeatherLocation().c_str()); }, 
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeyOpenWeatherLocation); },
        [this](configline& c){ return _system.settings().WeatherSource() == EnvironmentOpenWeather::name(); }, 
        [this](configline& c){ _environment.triggerUpdate(); });
    addConfig(ENG_WEATHER, 
        [this](configline& c){ generateWeatherLine(c); });
    addConfig(ENG_SEC, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeySmoothSecondHand, _secondhandChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeySmoothSecondHand, _secondhandChoices); });
    addConfig(ENG_FLIP_KEYS, 
        [this](configline& c){ generateSettingLine(c, AppSettings::KeyFlipKeys, _flipKeyChoices); }, 
        [this](configline& c, bool init){ return updateSettingChoices(c, init, AppSettings::KeyFlipKeys, _flipKeyChoices); });
    addConfig(ENG_UPDATE_PERIOD, 
        [this](configline& c){ generateIntervalLine(c); }, 
        [this](configline& c, bool init){ return updateSettingSwUpdate(c, init); });
    addConfig(ENG_UPDATE_URL, 
        [this](configline& c){ snprintf(c.value, sizeof(c.value), "%s", _system.settings().SoftwareUpdateURL().c_str()); },  
        [this](configline& c, bool init){ return updateSettingFreeText(c, init, AppSettings::KeySoftwareUpdateUrl); });
    addConfig(ENG_EXIT, 
        nullptr, 
        [this](configline&, bool){ _exitCode = 1; return false; });

    for (auto e : env.sources())
    {
        auto name = e->source();
        _weatherChoices.push_back(configchoice{name,name});
    }

    calculateLabelWidth();
}

void ConfigurationUI::init()
{
    _configYBase = 0.0f;
    _selectionYBase = 0.0f;
    _selectedLine = 0;
    _keySetLine= 0;
    _updating = false;
    _exitCode = 0;
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
    auto colUpdating = Color::white * _appctx.intensity();
    auto colAccept = Color::lime * _appctx.intensity();
    auto colWritable = Color::white * _appctx.intensity() * (_updating ? 0.5f : 1.0f);
    auto colSelect = Color(16,16,16) * _appctx.intensity();
    auto colReadonly = Color::lime * _appctx.intensity() * (_updating ? 0.5f : 1.0f);
    
    auto xvalues = _labelwidth + 2;

    // draw selection line background
    auto x = _updating ? xvalues : 0.0;
    auto dx = graphics.dx() - x;
    graphics.rect(x, _selectionYBase - _configYBase, dx, _font->height(), colSelect);

    // draw visible configs lines
    auto yt = - _configYBase;
    for (auto i=0; i<nConfigs(); ++i)
    {
        if (yt >= graphics.dy())
            break;

        auto &config = getConfig(i);
        if (!config.visible(config))
            continue;

        auto yb = yt + _font->height();
        if (yb >= 0)
        {
            auto y = _font->height() + _font->descend();

            // draw label
            auto x = _margin;
            auto view = graphics.clipOrigin(x, yt, _labelwidth, yb - yt);
            x = view.text(_font, config.xLabelScrollOffset, y, translate(config.label).c_str(), colWritable);  
            updateScrollState(config.xLabelScrollState, config.xLabelScrollOffset, config.xLabelScrollDelay, x < view.dx());

            // draw value
            auto editing = i == _selectedLine && _updating;
            auto textEditing = editing && _iEditIndex >= 0;
            if (!textEditing)
            {
                x = _margin + _labelwidth + 2;
                view = graphics.clipOrigin(x, yt, graphics.dx() - x, yb - yt);
                auto color = config.updater ? (editing ? colUpdating : colWritable) : colReadonly;
                x = view.text(_font, config.xValueScrollOffset, y, config.value, color);    
                updateScrollState(config.xValueScrollState, config.xValueScrollOffset, config.xValueScrollDelay, x < view.dx());
            }
        }
        yt += _font->height();
    }

    // draw textual editing interface (vertical character column)
    if (_iEditIndex >= 0)
    {
        auto neditchars = strlen(_editChars);
        auto &config =  getConfig(_selectedLine);

        auto x = config.xValueScrollOffset;
        auto xroll = x;

        // draw text and empty vertical character-roll
        auto yt = _selectedLine * _font->height() - _configYBase;
        auto yb = yt + _font->height();
        auto y = _font->height() + _font->descend() - 2;
        auto targetXScrollOffset = config.xValueScrollOffset;
        auto view = graphics.moveOrigin(xvalues, yt, graphics.dx() - x, yb - yt);
        for (auto i=0; i < sizeof(config.value); ++i)
        {
            auto c = config.value[i];
            if (i == _iEditIndex)
            {
                xroll = x;
                x += _rollXOffset;
                graphics.rect(xvalues + x, 0, _font->sizex(), graphics.dy(), colSelect);
                graphics.line(xvalues + x, 0, xvalues + x, graphics.dy(), 0.5, colUpdating);
                graphics.line(xvalues + x + _font->sizex() + 1, 0, xvalues + x + _font->sizex() + 1, graphics.dy(), 0.5f, colUpdating);
                x += _font->sizex() - _rollXOffset;
                targetXScrollOffset = config.xValueScrollOffset + std::min(0.0f, graphics.dx() - x - xvalues);
            }
            else
            {
                x = view.text(_font, x, y,  c, colUpdating);
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
        while (yt < graphics.dy())
        {
            auto y = yt;
            auto dx = _font->sizex();
            auto dy = _font->sizey();
            if (_editChars[iroll] == AcceptChar)
            {
                auto xc = x + xvalues;
                graphics.triangle(xc+1, y+3, xc+3, y+3, xc+dx/3, y+dy-1, colAccept);
                graphics.triangle(xc+dx-3, y+1, xc+dx, y+1, xc+dx/3, y+dy-1, colAccept);
            }
            else
            {
                auto w = _font->charsize(_editChars[iroll]).dx;
                y = yt + _font->height() + _font->descend() - 2;
                auto xc = xvalues + x + (_font->sizex() - w) / 2;
                graphics.text(_font, xc, y, _editChars[iroll], colUpdating);
            }
            yt += _font->height();
            iroll = (iroll + 1) % neditchars;
        }
        graduallyUpdateVariable(_rollYOffset, 0, 2);
        graduallyUpdateVariable(_rollXOffset, 0, 2);
        graduallyUpdateVariable(config.xValueScrollOffset, targetXScrollOffset, 2);
    }

    auto targetHi = _selectedLine * _font->height();
    auto targetLo = targetHi - graphics.dy() + _font->height();
    auto allPreSelectedLinesAreReadonly = true;
    for (auto i = 0; i < _selectedLine; i++)
    {
        allPreSelectedLinesAreReadonly &= getConfig(i).updater == nullptr;
    }
    if (allPreSelectedLinesAreReadonly)
    {
        targetLo = 0.0;
        targetHi = 0.0;
    }
    graduallyUpdateVariable(_configYBase, targetLo, targetHi, 1.0f);
    graduallyUpdateVariable(_selectionYBase, _selectedLine * _font->height(), 1.0f);
}

void ConfigurationUI::calculateLabelWidth()
{
    _labelwidth = 0.0f;
    for(auto &config : _configs)
    {
        auto info = _font->textsize(translate(config.label).c_str());
        _labelwidth = std::max(_labelwidth, info.dx);
    }
    _labelwidth = std::min(_labelwidth, _system.settings().PanelMode() == 1 ?  32.0f : 24.0f);
}

void ConfigurationUI::updateScrollState(ScrollState &scrollstate, float &scrolloffset, uint64_t &scrolldelay, bool endfits)
{
    switch (scrollstate)
    {
    case ScrollState::Begin:
        scrolloffset = 0;
        if (timeout(scrolldelay, 1000))
        {
            starttimer(scrolldelay);
            scrollstate = ScrollState::Scrolling;
        }
        break;
    case ScrollState::Scrolling:
        if (endfits)
        {
            scrollstate = ScrollState::End;
            starttimer(scrolldelay);
        }
        else
        {
            scrolloffset -= std::min(1.0f, (float)elapsed(scrolldelay / 8));
        }
        break;
    case ScrollState::End:
        if (timeout(scrolldelay, 1500))
        {
            scrollstate = ScrollState::Begin;
            starttimer(scrolldelay);
        }
        break;
    }
}

int ConfigurationUI::interact()
{
    {
        // iteratively update the dynamic value for each configline
        auto &config = getConfig(_inextReaderUpdate);
        if (config.reader && (!_updating || _inextReaderUpdate != _selectedLine))
        {
            config.reader(config);
        }
        _inextReaderUpdate = (_inextReaderUpdate + 1) % nConfigs();
    }

    auto &config = getConfig(_selectedLine);
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
        _keySetLine = std::max(0.0f, std::min(_keySetLine, nConfigs() - 1.0f));
        if ((int)_keySetLine != _selectedLine)
        {
            selectConfig((int)_keySetLine);
            _keySetLine = _selectedLine;
        }
    }

    if (isTimeout())
    {
        _exitCode = 1;
    }
    if (_exitCode != 0)
    {
        _system.settings().saveSettings();
        _system.connectWifi();
    }
    return _exitCode;
}

void ConfigurationUI::addConfig(const char *label,
    std::function<void(configline&)> reader,
    std::function<bool(configline&, bool)> updater,
    std::function<bool(configline&)> visible,
    std::function<void(configline&)> onexit)
{
    _configs.push_back(configline());
    auto &config = _configs.back();

    config.label = label;
    config.value[0] = 0;
    config.reader = reader;
    config.updater = updater;
    config.visible = visible != nullptr ? visible : [=](configline&){ return true; };
    config.onexitaction = onexit;
}

configline& ConfigurationUI::getConfig(int i)
{
    for(auto &config : _configs)
    {
        if (config.visible(config))
        {
            if (i-- == 0)
                return config;
        }
    }
    // should never reach
    return _configs[0];
}

int ConfigurationUI::nConfigs()
{
    auto n = 0;
    for(auto &config : _configs)
    {
        if (config.visible(config))
            n++;
    }
    return n;
}

void ConfigurationUI::selectConfig(int i)
{
    auto n = nConfigs();

    if (i < _selectedLine)
    {
        while (!getConfig(i).updater && i > 0 && i < n)
        {
            i--;
        }
    }
    else
    {
        while(!getConfig(i).updater && i >= 0 && (i + 1 < n))
        {
            i++;
        }
    }
    if (i>=0 && i < n && getConfig(i).updater)
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

    auto editing = !isEditCancelled();
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

    auto editing = !isEditCancelled();
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

    auto editing = !isEditCancelled();
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

    auto editing = !isEditCancelled();
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

    auto editing = !isEditCancelled();
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

bool ConfigurationUI::updateSettingInteger(configline& config, bool init, const char *settingKey, int min, int max)
{
    if (init)
    {
        config.setpoint = _system.settings().get(settingKey)->asint();
    }

    auto editing = !isEditCancelled();
    if (editing)
    {
        auto key = getKey();
        editing = key != UserInput::KEY_SET;
        if (editing)       
        {
            repeatUpdateOnKey(UserInput::KEY_DOWN, key, [&](float delta){ config.setpoint += delta; });
            repeatUpdateOnKey(UserInput::KEY_UP, key, [&](float delta){ config.setpoint -= delta; });
        }
        if (config.setpoint < min)
            config.setpoint = min;
        else if (config.setpoint >= max)
            config.setpoint = max;
        
        auto integer = (int)config.setpoint;
        snprintf(config.value, sizeof(config.value), "%d:%02d", integer / 60, integer % 60);

        if (!editing)
        {
            _system.settings().get(settingKey)->set(integer);
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

    auto editing = !isEditCancelled();
    if (editing)
    {
        auto key = getKey();
        auto idx = (int)config.setpoint;
        switch (key)
        {
            case UserInput::KEY_SET:
                _system.settings().get(settingKey)->set(choices[idx].store);
                calculateLabelWidth();
                editing = false;
                break;
            case UserInput::KEY_UP:
                config.xValueScrollState = ScrollState::End;
                config.xValueScrollOffset = 0;
                starttimer(config.xLabelScrollDelay);
                idx++;
                break;
            case UserInput::KEY_DOWN:
                config.xValueScrollState = ScrollState::End;
                config.xValueScrollOffset = 0;
                starttimer(config.xLabelScrollDelay);
                idx--;
                break;
        }

        idx = (idx + choices.size()) % choices.size();
        config.setpoint = idx;
        snprintf(config.value, sizeof(config.value), "%s", translate(choices[idx].english).c_str());
    }
    return editing;
}

void ConfigurationUI::generateIntervalLine(configline &config)
{
    auto interval = _system.settings().get(AppSettings::KeySoftwareUpdateInterval)->asint();
    for (auto i=0; i<_swUpdateChoices.size(); ++i)
    {
        if (interval <= _swUpdateChoices[i].interval) 
        {
            snprintf(config.value, sizeof(config.value), "%s", translate(_swUpdateChoices[i].english).c_str());
            config.setpoint = i;
            break;
        }
    }
}

bool ConfigurationUI::updateSettingSwUpdate(configline& config, bool init)
{
    if (init)
    {
        // setpoint already set in generateIntervalLine()
    }

    auto editing = !isEditCancelled();
    if (editing)
    {
        auto key = getKey();
        switch (key)
        {
            case UserInput::KEY_SET:
                config.setpoint = _swUpdateChoices[(int)config.setpoint].interval;
                _system.settings().get(AppSettings::KeySoftwareUpdateInterval)->set((int)config.setpoint);
                editing = false;
                break;
            case UserInput::KEY_UP:
                if (config.setpoint + 1 < _swUpdateChoices.size())
                {
                    config.setpoint++;
                }
                break;
            case UserInput::KEY_DOWN:
                if (config.setpoint > 0)
                {
                    config.setpoint--;  
                }
                break;
        }
        if (editing)
        {
            snprintf(config.value, sizeof(config.value), "%s", translate(_swUpdateChoices[config.setpoint].english).c_str());
        }
    }
    return editing;
}

int ConfigurationUI::getKey() 
{ 
    auto keypress = _userinput.getKeyPress(); 
    if (keypress.key != 0)
    {
        _lastEditTime = _system.now();
    }
    return keypress.key;
}

KeyPress ConfigurationUI::getKeyPress() 
{ 
    auto keypress = _userinput.getKeyPress(); 
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

bool ConfigurationUI::isEditCancelled()
{
    return _userinput.hasKeyDown(UserInput::KEY_SET, 1500);
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

    auto step = (_system.now().msticks() / 1000) % 4;
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
    else if (_environment.isUpdating())
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
    config.xValueScrollOffset = 0;
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
