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
    enum class ParseState { WaitArray, WaitObject1, Reading, Completed };
    ParseState _state;

    bool _valid;
    optional<uint64_t> _timestamp;
    optional<tm> _sunset;
    optional<tm> _sunrise;
    optional<float> _temperature;
    optional<float> _windchill;
    optional<weathertype> _weather;
    optional<float> _windangle;
    optional<float> _windspeed;
    optional<float> _airpressure;

public:
    /// @param accesskey the accesskey to their API, request one at their site.
    /// @param location the location name you want the info for. 
    /// E.g. "Amsterdam" or longitude/lattitude longitude e.g. "52.0910879,5.1124231"
    EnvironmentWeerlive(System *system, Setting *key, Setting *location)
    {
        _system = system;
        _accesskey = key;
        _location = location;
        _valid = false;
    }

    void updateTask();

    bool valid() const { return _valid; };
    optional<tm> sunset() const { return _sunset; }
    optional<tm> sunrise() const { return _sunrise; }
    optional<float> temperature() const { return _temperature; }
    optional<float> windchill() const { return _windchill; }
    optional<weathertype> weather() const { return _weather; }
    optional<float> windangle() const { return _windangle; }
    optional<float> windspeed() const { return _windspeed; };
    optional<float> airpressure() const { return _airpressure; };

private:
    bool handleJson(const JsonEntry &json);
    weathertype parseWeather(const char *image);
    tm parseTime(const char *s);
    void clearValues();
};

#endif
