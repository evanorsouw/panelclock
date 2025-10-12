
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <algorithm>
#include <functional>
#include <map>
#include <string>

class Settings;

class Setting
{
private:
    Settings *_container;
    std::string _name;
    std::string _value;

public:
    Setting(Settings* container, std::string name, const char *v) : _container(container), _name(name) { set(v); }
    Setting(Settings* container, std::string name, int v) : _container(container), _name(name) { set(v); }
    Setting(Settings* container, std::string name, bool v) : _container(container), _name(name) { set(v); }

    void set(const char * v) { _value = v; onChanged(); }
    void set(int v) { _value = std::to_string(v); onChanged(); }
    void set(int64_t v) { _value = std::to_string(v); onChanged(); }
    void set(bool v) { _value = v ? "1" : "0"; onChanged(); }

    const std::string &name() { return _name; }
    bool asbool() const { return _value == "1"; }
    int asint() const { return atoi(_value.c_str()); }
    int64_t asint64() const { return std::stoll(_value.c_str()); }
    const std::string &asstring() { return _value; }

private:
    void onChanged();
};

class Settings
{
friend Setting;
private:
    std::string _nvsname;
    std::map<std::string, Setting *> _settings;
    std::vector<std::function<void(Setting*)>> _onChangedCallbacks;

public:
    Settings(const char *nvsname) : _nvsname(nvsname) {}
    virtual ~Settings() {}

    bool loadSettings();
    bool saveSettings();

    void onChanged(std::function<void(Setting*)> callback, bool informImmediately = false);

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

private:
    void onChanged(Setting *setting)
    {
        std::for_each(_onChangedCallbacks.begin(), _onChangedCallbacks.end(), [=](std::function<void(Setting*)> cb){ cb(setting); });
    }
};

inline void Setting::onChanged()
{
    _container->onChanged(this);
}

#endif
