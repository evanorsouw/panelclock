
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
    int asint() const { return atoi(_value.c_str()); }
    const char *asstring() const { return _value.c_str(); }
};

class Settings
{
private:    
    std::map<std::string, Setting *> _settings;

public:
    Settings() {}
    virtual ~Settings() {}

    bool loadSettings();
    bool saveSettings();

    Setting *add(const char *name, bool value) { return add(name, value ? "1" : "0"); }
    Setting *add(const char *name, int value) 
    { 
        char buf[40];
        snprintf(buf, sizeof(buf), "%d", value); 
        return add(name, buf); 
    }
    Setting *add(const char *name, const char *value); 
    
    Setting *get(const char *name) const;
    Setting *get(const char *name, bool defaultValue) { return get(name, defaultValue ? "1": "0"); }
    Setting *get(const char *name, int defaultValue) 
    { 
        char buf[40];
        snprintf(buf, sizeof(buf), "%d", defaultValue); 
        return get(name, buf);
    }
    Setting *get(const char *name, const char *defaultValue);
};

#endif
