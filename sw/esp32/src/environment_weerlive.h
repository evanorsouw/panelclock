#ifndef _ENVIRONMENT_WEERLIVE_H_
#define _ENVIRONMENT_WEERLIVE_H_

#include "environment.h"
#include "jsonparser.h"
#include "settings.h"
#include "system.h"

class EnvironmentWeerlive : public Environment
{
private:
    System *_system;
    Setting *_accesskey;
    Setting *_location;
    enum class ParseState { WaitArray, WaitObject1, Reading, Completed, Failed };
    ParseState _state;
    optional<uint64_t> _timestamp;
    EnvironmentValues _parsedValues;
    EnvironmentValues _values;
    bool _updating;
    timeinfo _lastupdate;
    EventGroupHandle_t _eventGroup;

public:
    /// @param accesskey the accesskey to their API, request one at their site.
    /// @param location the location name you want the info for. 
    /// E.g. "Amsterdam" or longitude/lattitude longitude e.g. "52.0910879,5.1124231"
    EnvironmentWeerlive(System *system, Setting *key, Setting *location)
    {
        _system = system;
        _accesskey = key;
        _location = location;
        _parsedValues.clear();
        _updating = false;
        _eventGroup = xEventGroupCreate();
        _lastupdate = system->now();
    }

    int update();
    timeinfo lastupdate() const { return _lastupdate; }

    bool isupdating() const { return _updating; }
    std::string invalidReason() const { return _values.invalidReason; };
    optional<std::string> location() const { return _values.location; }
    optional<tm> sunset() const { return _values.sunset; }
    optional<tm> sunrise() const { return _values.sunrise; }
    optional<float> temperature() const { return _values.temperature; }
    optional<float> windchill() const { return _values.windchill; }
    optional<weathertype> weather() const { return _values.weather; }
    optional<float> windangle() const { return _values.windangle; }
    optional<float> windspeed() const { return _values.windspeed; };
    optional<float> airpressure() const { return _values.airpressure; };

private:
    bool handleJson(const JsonEntry &json);
    weathertype parseWeather(const char *image);
    float parseWindr(const char *r);
    tm parseTime(const char *s);
};

#endif
