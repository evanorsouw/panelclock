#ifndef _ENVIRONMENT_WEERLIVE_H_
#define _ENVIRONMENT_WEERLIVE_H_

#include "environment.h"
#include "jsonparser.h"
#include "settings.h"
#include "system.h"

class EnvironmentWeerlive : public EnvironmentBase
{
private:
    System *_system;
    Setting *_accesskey;
    Setting *_location;
    enum class ParseState { WaitArray, WaitObject1, Reading, Completed, Failed };
    ParseState _state;
    std::string _weatherImage;
    optional<uint64_t> _timestamp;
    EnvironmentValues _parsedValues;
    EnvironmentValues _values;
    bool _updating;
    timeinfo _lastupdate;
    TimerHandle_t _hTimer;
    const int _updateIntervalTicks = pdMS_TO_TICKS(10*60*1000); // every 10 minutes

public:
    /// @param accesskey the accesskey to their API, request one at their site.
    /// @param location the location name you want the info for. 
    /// E.g. "Amsterdam" or longitude/lattitude longitude e.g. "52.091,5.112"
    EnvironmentWeerlive(System *system, Settings &settings, Event *event);

    static const char *name() { return "weerlive"; }

    void update() override;
    bool isUpdating() const { return _updating; }
    timeinfo lastUpdate() const { return _lastupdate; }

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
    bool handleJson(const JsonEntry &json);
    std::vector<WeatherLayer> parseWeather(const char *image);
    float parseWindr(const char *r);
    tm parseTime(const char *s);
};

#endif
