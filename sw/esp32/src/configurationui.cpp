
#include <algorithm>
#include "configurationui.h"
#include "timeinfo.h"

ConfigurationUI::ConfigurationUI(Graphics &graphics, Environment &env, System &sys, UserInput &userinput, Font *font)
    : _graphics(graphics)
    , _env(env)
    , _sys(sys)
    , _userinput(userinput)
{
    _font= font;

    addConfig("wifi", 
        [this](configline&c, bool init){ if (init) _sys.scanAPs(); else _sys.connectWifi(); },
        [this](configline&c){ strcpy(c.value,_sys.settings().WifiSid()->asstring()); }, 
        [this](configline &config, bool init){ return updateWifiSid(config, init); });
    addConfig("pass", 
        nullptr,
        [this](configline&c){ strcpy(c.value,_sys.settings().WifiPassword()->asstring()); }, 
        [this](configline &config, bool init){ return updateWifiPassword(config, init); });
    addConfig("ip", 
        nullptr, 
        [this](configline&c){ generateWifiLine(c); },
        nullptr);
    addConfig("year", 
        nullptr, 
        [this](configline&c){ sprintf(c.value, "%04d", _sys.now().year()); }, 
        [this](configline &config, bool init){ return updateYear(config, init); });
    addConfig("date", 
        nullptr, 
        [this](configline&c){ sprintf(c.value,"%02d %s", _sys.now().mday(), _sys.now().monthName(false)); }, 
        [this](configline &config, bool init){ return updateDate(config, init); });
    addConfig("time", 
        nullptr, 
        [this](configline&c){ sprintf(c.value,"%02d:%02d:%02d", _sys.now().hour(), _sys.now().min(), _sys.now().sec()); },  
        [this](configline &config, bool init){ return updateTime(config, init); });
    addConfig("exit", 
        nullptr, 
        nullptr, 
        [this](configline&, bool){ _exitConfig = true; return false; });
}

void ConfigurationUI::startConfigurationSession()
{
    _configYBase = 0.0f;
    _selectionYBase = 0.0f;
    _selectedLine = 0;
    _updating = false;
    _exitConfig = false;
    _lastEditTime = _sys.now();
}

void ConfigurationUI::endConfigurationSession()
{    
}

bool ConfigurationUI::render(Bitmap &screen)
{
    drawConfigLines(screen);

    for (auto i=0; i<_configs.size(); i++)
    {
        auto &config = _configs[i];
        if (config.reader && (!_updating || i != _selectedLine))
        {
            config.reader(_configs[i]);
        }
    }

    auto &config = _configs[_selectedLine];
    if (_updating)
    {        
        _updating = config.updater(config, false);
    }
    else
    {
        switch (getKey())
        {
        case UserInput::KEY_SET:
            config.updater(config, true);
            _updating = true;
            break;
        case UserInput::KEY_UP:
            selectConfig(_selectedLine-1);
            break;
        case UserInput::KEY_DOWN:
            selectConfig(_selectedLine+1);
            break;
        }
    }

    return _exitConfig || isTimeout();
}

void ConfigurationUI::addConfig(const char *label,
    std::function<void(configline &, bool)> initter,
    std::function<void(configline &)> reader,
    std::function<bool(configline &, bool)> updater)
{
    _configs.push_back(configline());
    auto &config = _configs.back();

    config.label = label;
    config.value[0] = 0;
    config.initter = initter;
    config.reader = reader;
    config.updater = updater;
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
        runInitter(false);
        _selectedLine = i;
        printf("selectedline=%d\n", _selectedLine);
        runInitter(true);
    }        
}

void ConfigurationUI::runInitter(bool init)
{
    auto &config = _configs[_selectedLine];
    if (config.initter)
    {
        config.initter(config, init);
    }
}

void ConfigurationUI::drawConfigLines(Bitmap &screen)
{
    auto x = _updating ? 29 : 0;
    auto dx = _updating ? 128 - 29 : 128;
    _graphics.rect(screen, x, _selectionYBase - _configYBase, dx, _font->height(), Color(19,0,0));

    for (auto i=0; i<_configs.size(); ++i)
    {
        auto &info = _configs[i];
        auto yt = i * _font->height() - _configYBase;
        auto yb = yt + _font->height();

        if (yb < 0 || yt >= 64)
            continue;

        auto y = yt + _font->height() + _font->descend() - 2;
        _graphics.text(screen, _font, 2, y, info.label, Color::white);
        _graphics.text(screen, _font, 30, y, info.value, Color::white);
    }
    float dy = 0.0f;
    bool down = false;
    auto targety = _selectedLine * _font->height() - _configYBase;
    if (targety < 0)
    {
        dy = -targety;
        down = false;
    }
    else if (targety  > 64 - _font->height())
    {
        dy = targety - (64 - _font->height());
        down = true;
    }
    if (dy > 0.0f)
    {
        auto delta = std::min(dy, std::log(dy + 1));
        _configYBase = _configYBase + (down ? delta : -delta);
    }

    targety = _selectedLine * _font->height() - _selectionYBase;
    dy = std::abs(targety);
    down = (targety > 0);
    if (dy > 0.0f)
    {
        auto delta = std::min(dy, std::log(dy + 1));
        _selectionYBase += down ? delta : -delta;
    }
}

bool ConfigurationUI::updateYear(configline &config, bool init)
{
    if (init)
    {
        config.start =_sys.now();        
        config.setpoint = config.start.year();
    }

    if (isEditTimeout())
        return false;
        
    auto key = getKey();
    if (key == UserInput::KEY_SET)
    {
        _sys.now(config.start);
        return false;
    }
    repeatUpdateOnKey(UserInput::KEY_DOWN, key, [&](float delta){ config.setpoint += delta; });
    repeatUpdateOnKey(UserInput::KEY_UP, key, [&](float delta){ config.setpoint -= delta; });

    config.setpoint = std::max(1901.0f, std::min(2099.0f, config.setpoint));
    config.start.setDate((int)config.setpoint, config.start.mon(), config.start.mday());
    sprintf(config.value, "%04d", config.start.year());

    return true;
}

bool ConfigurationUI::updateDate(configline &config, bool init)
{
    if (init)
    {
        config.start =_sys.now();        
        config.setpoint = config.start.yday();
    }

    if (isEditTimeout())
        return false;
                
    auto key = getKey();
    if (key == UserInput::KEY_SET)
    {
        _sys.now(config.start);
        return false;
    }
    repeatUpdateOnKey(UserInput::KEY_DOWN, key, [&](float delta){ config.setpoint += delta; });
    repeatUpdateOnKey(UserInput::KEY_UP, key, [&](float delta){ config.setpoint -= delta; });

    auto hasleap = (config.start.year() % 4 == 0) && (config.start.year() % 400 == 0 || config.start.year() % 100 != 0);
    auto maxdays = hasleap ? 366 : 365;
    auto yday = (int)config.setpoint % maxdays + 1;
    auto month = 0;
    auto mday = yday;

    while (mday > timeinfo::daysInMonth(month, hasleap))
    {
        mday -= timeinfo::daysInMonth(month, hasleap);
        month++;
    }
    printf("=> yday=%d, month=%d, mday=%d\n", yday, month, mday);

    config.start.setDate(config.start.year(), month, mday);
    sprintf(config.value, "%02d %s", config.start.mday(), config.start.monthName(false));

    return true;
}

bool ConfigurationUI::updateTime(configline &config, bool init)
{
    if (init)
    {
        config.start =_sys.now();        
        config.start.sec(0);
        config.start.addMinutes(1);
        config.setpoint = config.start.hour() * 60 + config.start.min();
    }

    if (isEditTimeout())
        return false;
        
    auto key = getKey();
    if (key == UserInput::KEY_SET)
    {
        _sys.now(config.start);
        return false;
    }
    repeatUpdateOnKey(UserInput::KEY_DOWN, key, [&](float delta){ config.setpoint += delta; });
    repeatUpdateOnKey(UserInput::KEY_UP, key, [&](float delta){ config.setpoint -= delta; });

    config.start.hour((int)config.setpoint / 60);
    config.start.min((int)config.setpoint % 60);

    auto &start = config.start;
    sprintf(config.value, "%02d:%02d:%02d", start.hour(), start.min(), start.sec());

    return true;
}

bool ConfigurationUI::updateWifiSid(configline &config, bool init)
{
    if (init)
    {
    }

    auto key = getKey();

    if (isEditTimeout() || key == UserInput::KEY_SET)
    {
        return false;
    }

    return true;
}

bool ConfigurationUI::updateWifiPassword(configline &config, bool init)
{
    if (init)
    {
    }

    if (isEditTimeout())
        return false;
        
    auto key = getKey();
    if (key == UserInput::KEY_SET)
    {
        return false;
    }

    return true;
}


int ConfigurationUI::getKey() 
{ 
    auto key = _userinput.getKey(); 
    if (key != 0)
    {
        _lastEditTime = _sys.now();
    }
    return key;
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
    return _userinput.hasKeyDown(UserInput::KEY_SET, 3000);
}

bool ConfigurationUI::isTimeout()
{
    auto elapsed = _sys.now().msticks() - _lastEditTime.msticks();
    if (elapsed < 0)
    {
        _lastEditTime = _sys.now();
    }
    return elapsed > 120000;
}

void ConfigurationUI::generateWifiLine(configline &config)
{
    if (_sys.wifiConnected()) 
    {
        strcpy(config.value, _sys.IPAddress()); 
        return;
    }

    auto label = "";
    if (_sys.wifiScanning()) 
    {
        label = "scanning";
    }
    else if (_sys.wifiConnecting()) 
    {
        label = "connecting";
    }
    else
    {
        config.value[0] = 0;
        return;
    }
    auto step = (_sys.now().msticks() / 800) % 4;
    char steps[5] = "    ";
    steps[step] = '.';
    sprintf(config.value, "%s%s(%d)", label, steps, _sys.nAPs());
}