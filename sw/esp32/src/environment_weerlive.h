#ifndef _ENVIRONMENT_WEERLIVE_H_
#define _ENVIRONMENT_WEERLIVE_H_

#include "environment.h"
#include "jsonparser.h"
#include "settings.h"

class EnvironmentWeerlive : public Environment
{
private:
    Setting *_accesskey;
    Setting *_location;
    enum class ParseState { WaitArray, WaitObject1, Reading, Completed };
    ParseState _state;

    bool _valid;
    uint64_t _timestamp;
    tm _sunset;
    tm _sunrise;
    float _temperature;
    float _windchill;
    weathertype _weather;
    float _windangle;
    float _windspeed;
    float _airpressure;

public:
    /// @param accesskey the accesskey to their API, request one at their site.
    /// @param location the location name you want the info for. 
    /// E.g. "Amsterdam" or longitude/lattitude longitude e.g. "52.0910879,5.1124231"
    EnvironmentWeerlive(Setting *key, Setting *location)
    {
        _accesskey = key;
        _location = location;
        _valid = false;
    }

    void update();

    bool valid() const { return _valid; };
    tm sunset() const { return _sunset; }
    tm sunrise() const { return _sunrise; }
    float temperature() const { return _temperature; }
    float windchill() const { return _windchill; }
    weathertype weather() const { return _weather; }
    float windangle() const { return _windangle; }
    float windspeed() const { return _windspeed; };
    float airpressure() const { return _airpressure; };

private:
    bool handleJson(const JsonEntry &json);
    weathertype parseWeather(const char *image);
    tm parseTime(const char *s);
};

#endif
