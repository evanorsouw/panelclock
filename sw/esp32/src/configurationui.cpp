
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

    addConfig("DST", 
        nullptr,
        [this](configline&c){ generateDSTLine(c, _sys.settings().DST()); }, 
        [this](configline &config, bool init){ return updateDST(config, init); });
    addConfig(translate("year"), 
        nullptr, 
        [this](configline&c){ snprintf(c.value, sizeof(c.value), "%04d", _sys.now().year()); }, 
        [this](configline &config, bool init){ return updateYear(config, init); });
    addConfig(translate("date"), 
        nullptr, 
        [this](configline&c){ snprintf(c.value, sizeof(c.value), "%02d %s", _sys.now().mday(), _sys.now().monthName(false)); }, 
        [this](configline &config, bool init){ return updateDate(config, init); });
    addConfig(translate("time"), 
        nullptr, 
        [this](configline&c){ snprintf(c.value, sizeof(c.value), "%02d:%02d:%02d", _sys.now().hour(), _sys.now().min(), _sys.now().sec()); },  
        [this](configline &config, bool init){ return updateTime(config, init); });
    addConfig(translate("wifi"), 
        [this](configline&c, bool init){ if (init) _sys.scanAPs(); else _sys.connectWifi(); },
        [this](configline&c){ strcpy(c.value,_sys.settings().WifiSid()); }, 
        [this](configline &config, bool init){ return updateWifiSid(config, init); });
    addConfig(translate("pass"), 
        nullptr,
        [this](configline&c){ strcpy(c.value,_sys.settings().WifiPassword()); }, 
        [this](configline &config, bool init){ return updateWifiPassword(config, init); });
    addConfig(translate("ip"), 
        nullptr, 
        [this](configline&c){ generateWifiLine(c); },
        nullptr);
    addConfig(translate("exit"), 
        nullptr, 
        nullptr, 
        [this](configline&, bool){ _exitConfig = true; return false; });   

    _labelwidth = 0;
    for(auto &config : _configs)
    {
        auto info = _font->textsize(config.label);
        _labelwidth = std::max(_labelwidth, info.dx);
    }
}

void ConfigurationUI::startConfigurationSession()
{
    _configYBase = 0.0f;
    _selectionYBase = 0.0f;
    _selectedLine = 0;
    _updating = false;
    _exitConfig = false;
    _lastEditTime = _sys.now();
    selectConfig(0);
}

void ConfigurationUI::endConfigurationSession()
{    
    _sys.connectWifi();
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
        _lastEditTime = _sys.now();
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
    auto margin = 2.0f;
    auto x = _updating ? _labelwidth + margin * 2 : 0;
    auto dx = _updating ? 128 - _labelwidth - margin * 2 : 128;
    _graphics.rect(screen, x, _selectionYBase - _configYBase, dx, _font->height(), Color(19,0,0));

    for (auto i=0; i<_configs.size(); ++i)
    {
        auto &info = _configs[i];
        auto yt = i * _font->height() - _configYBase;
        auto yb = yt + _font->height();
        auto color = info.updater ? Color::white : Color::lime;

        if (yb < 0 || yt >= 64)
            continue;

        auto y = yt + _font->height() + _font->descend() - 2;
        _graphics.text(screen, _font, margin, y, info.label, Color::white);
        _graphics.text(screen, _font, _labelwidth + margin * 2, y, info.value, color);
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

bool ConfigurationUI::updateDST(configline &config, bool init)
{
    if (init)
    {
        config.setpoint = _sys.settings().DST() ? 1 : 0;
    }

    auto editing = !isEditTimeout();
    if (editing)
    {
        auto key = getKey();
        editing = key != UserInput::KEY_SET;
        auto dst = (int)config.setpoint;
        if (editing)
        {
            switch (key)
            {
                case UserInput::KEY_DOWN:
                case UserInput::KEY_UP:
                    dst = !dst;
                    break;
            }
        }
        config.setpoint = dst ? 1 : 0;
        generateDSTLine(config, dst);

        if (!editing)
        {
            printf("writing dst: %d\n", dst);
            _sys.settings().DST(dst);
        }
    }
    return editing;
}

bool ConfigurationUI::updateYear(configline &config, bool init)
{
    if (init)
    {
        config.setpoint = _sys.now().year();
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
        auto now = _sys.now();
        now.setDate((int)config.setpoint, now.mon(), now.mday());
        snprintf(config.value, sizeof(config.value), "%04d", now.year());

        if (!editing)
        {
            _sys.now(now);
        }
    }
    return editing;
}

bool ConfigurationUI::updateDate(configline &config, bool init)
{
    if (init)
    {
        config.setpoint = _sys.now().yday();
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
        auto now = _sys.now();
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
        snprintf(config.value, sizeof(config.value),  "%02d %s", now.mday(), now.monthName(false));
        
        if (!editing)
        {
            _sys.now(now);
        }
    }
    return editing;
}

bool ConfigurationUI::updateTime(configline &config, bool init)
{
    if (init)
    {
        auto now =_sys.now();        
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
        
        auto now = _sys.now();
        now.setTime((int)config.setpoint / 60, (int)config.setpoint % 60, 0);
        snprintf(config.value, sizeof(config.value), "%02d:%02d:%02d", now.hour(), now.min(), now.sec());

        if (!editing)
        {
            _sys.now(now);
        }
    }
    return editing;
}

bool ConfigurationUI::updateWifiSid(configline &config, bool init)
{
    if (isEditTimeout())
        return false;

    auto naps = _sys.nAPs();

    auto key = getKey();
    auto idx = (int)config.setpoint;
    auto changed = false;
    switch (key)
    {
        case UserInput::KEY_SET:
            return false;
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
        snprintf(config.value, sizeof(config.value), "%s", _sys.APSID(idx));
        _sys.settings().WifiSid(config.value);
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
    return elapsed > 300*1000;
}

void ConfigurationUI::generateDSTLine(configline &config, bool dst)
{
    const char *txt = dst
        ? translate("summer") 
        : translate("winter");
    snprintf(config.value, sizeof(config.value), txt);
}

void ConfigurationUI::generateWifiLine(configline &config)
{
    if (_sys.wifiConnected()) 
    {
        snprintf(config.value, sizeof(config.value), "%s", _sys.IPAddress()); 
        return;
    }

    auto step = (_sys.now().msticks() / 400) % 4;
    char dots[5] = "    ";
    dots[step] = '.';
    if (_sys.wifiScanning()) 
    {
        snprintf(config.value, sizeof(config.value), "%s%s(%d)", translate("scanning"), dots, _sys.nAPs());
    }
    else if (_sys.wifiConnecting()) 
    {
        snprintf(config.value, sizeof(config.value), "%s%s", translate("connecting"), dots);
    }
    else
    {
        strcpy(config.value, "");
    }
}