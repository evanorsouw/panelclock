#ifndef _ENVIRONMENT_OPENWEATHER_H_
#define _ENVIRONMENT_OPENWEATHER_H_

#include "environment.h"
#include "settings.h"
#include "system.h"

class EnvironmentOpenWeather : public EnvironmentBase
{
private:
    System *_system;
    Setting *_accesskey;
    Setting *_location;
    optional<uint64_t> _timestamp;
    EnvironmentValues _parsedValues;
    EnvironmentValues _values;
    bool _updating;
    timeinfo _lastupdate;

public:
    /// @param accesskey the accesskey to their API, request one at their site.
    /// @param location the location name you want the info for. 
    /// E.g. "Amsterdam" or longitude/lattitude longitude e.g. "52.0910879,5.1124231"
    EnvironmentOpenWeather(System *system, Settings &settings, Event *updateEvent)
        : EnvironmentBase(name(), updateEvent)
    {
        _system = system;
        _accesskey = settings.get(AppSettings::KeyOpenWeatherKey);
        _location = settings.get(AppSettings::KeyOpenWeatherLocation);
        _parsedValues.clear();
        _updating = false;
        _lastupdate = system->now();
    }

    static const char *name() { return "openweather"; }

    void update() override;
    bool isUpdating() const override { return _updating; }
    timeinfo lastUpdate() const override { return _lastupdate; }

    std::string invalidReason() const override { return _values.invalidReason; };
    optional<std::string> location() const override { return _values.location; }
    optional<tm> &sunset() override { return _values.sunset; }
    optional<tm> &sunrise() override { return _values.sunrise; }
    optional<float> temperature() const override { return _values.temperature; }
    optional<float> windchill() const override { return _values.windchill; }
    optional<std::vector<WeatherLayer>> weather() const override { return _values.weather; }
    optional<float> windangle() const override { return _values.windangle; }
    optional<float> windspeed() const override { return _values.windspeed; };
    optional<float> airpressure() const override { return _values.airpressure; };

private:
    bool parseJson(const char *buf);
    tm parseTime(time_t when);
    float parseWindAngle(float angle);
    std::vector<WeatherLayer> parseWeather(int id);
};

#endif
