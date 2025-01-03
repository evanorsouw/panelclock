#ifndef _ENVIRONMENTSELECTOR_H_
#define _ENVIRONMENTSELECTOR_H_

#include "appsettings.h"
#include "environment.h"

class EnvironmentSelector : public IEnvironment
{
private:
    Settings &_settings;
    std::vector<EnvironmentBase*> _environments;
    EnvironmentBase *_active;

public:
    EnvironmentSelector(std::vector<EnvironmentBase*> environments, AppSettings &settings)
        : _settings(settings)
    {
        _environments = environments;
        _active = environments[0];
        selectEnvironment(settings.WeatherSource());

        _settings.onChanged([this](Setting *s) {
            if (s->name() == AppSettings::KeyWeatherSource)
            {
                selectEnvironment(s->asstring());
            }
        });
    }
    std::vector<EnvironmentBase*> sources() const { return _environments; }
    const char *source() const { return _active->source(); }
    bool valid() { return _active->valid(); }
    
    Event *triggerUpdateEvent() { return _active->triggerUpdateEvent(); }
    void triggerUpdate() { return _active->triggerUpdate(); }
    void update() { _active->update(); }
    bool isUpdating() const { return _active->isUpdating(); }
    timeinfo lastUpdate() const { return _active->lastUpdate(); }

    std::string invalidReason() const { return _active->invalidReason(); }
    optional<std::string> location() const { return _active->location(); }
    optional<tm> &sunset() { return _active->sunset(); }
    optional<tm> &sunrise() { return _active->sunrise(); }
    optional<float> temperature() const { return _active->temperature(); }
    optional<float> windchill() const  { return _active->windchill(); }
    optional<std::vector<WeatherLayer>> weather() const  { return _active->weather(); }
    optional<float> windangle() const  { return _active->windangle(); }
    optional<float> windspeed() const  { return _active->windspeed(); }
    optional<float> airpressure() const  { return _active->airpressure(); }

private:
    void selectEnvironment(const std::string& source)
    {
        auto it = std::find_if(_environments.begin(), _environments.end(), [source](IEnvironment *env) { 
            return env->source() == source; 
        });
        if (it != _environments.end())
        {
            _active = *it;
            printf("selected environment='%s'\n", _active->source());
            _active->triggerUpdate();
        }
    }
};

#endif