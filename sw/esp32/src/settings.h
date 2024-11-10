
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <algorithm>
#include <map>
#include <string>

class Setting
{
private:
    std::string _name;
    std::string _value;

public:
    Setting(std::string name, const char *v) : _name(name) { set(v); }
    Setting(std::string name, bool v) : _name(name) { set(v); }

    void set(const char * v) { _value = v; }
    void set(bool v) { _value = v ? "1" : "0"; }

    std::string name() const { return _name; }
    bool asbool() const { return _value == "1"; }
    const char *asstring() const { return _value.c_str(); }
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

    Setting *addSetting(const char *name, bool value) { return addSetting(name, value ? "1" : "0"); }
    Setting *addSetting(const char *name, const char *value); 
    
    Setting *getSetting(const char *name) const;

private:
    void readSettingFromNVS(Setting *setting);
};

#endif
