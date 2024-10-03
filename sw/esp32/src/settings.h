
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <algorithm>
#include <map>
#include <string>

class Setting
{
private:
    std::string _name;
    union {
        const char *s;
        float f;
        bool b;
    } _value;

public:
    Setting(std::string name, const char *v) : _name(name) { _value.s =v; }
    Setting(std::string name, float v) : _name(name) { _value.f = v; }
    Setting(std::string name, bool v) : _name(name) { _value.b = v; }

    void set(float v) { _value.f = v; }
    void set(const char * v) { _value.s = v; }
    void set(bool v) { _value.b = v; }

    std::string name() const { return _name; }
    float asfloat() const { return _value.f; }
    bool asbool() const { return _value.b; }
    const char *asstring() const { return _value.s; }
};

class Settings
{
private:    
    std::map<std::string, Setting *> _settings;

public:
    Settings();
    virtual ~Settings() {}

    bool loadSettings();
    bool saveSettings();

    template <class T>
    Setting *addSetting(const char *name, T value) 
    {
        Setting *setting;
        auto it = std::find_if(_settings.begin(), _settings.end(), [&](std::pair<std::string, Setting*> kv) { return kv.first == name; });
        if (it == _settings.end())
        {
            setting = new Setting(name, value);
            _settings[name] = setting;            
        }
        else
        {
            setting = it->second;
            setting->set(value);
        }
        return setting;
    }    
    Setting *getSetting(const char *name) const 
    { 
        auto it = _settings.find(name);
        if (it == _settings.end())
            return nullptr;
        return it->second;
    }
};

#endif
