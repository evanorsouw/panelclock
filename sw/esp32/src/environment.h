
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <string>
#include <time.h>
#include <vector>

#include "events.h"
#include "timeinfo.h"

enum class WeatherIcon
{
    Cloud,
    CloudWithLightRain,
    CloudWithHeavyRain,
    CloudWithHail,
    CloudWithLightning,
    SunWithRays,
    SunWithoutRays,
    Moon,
    Fog,
    SnowFlakes,
    Stars
};

struct WeatherLayer
{
    WeatherIcon icon;
    uint32_t color1;
    uint32_t color2;
    float ox;
    float oy;
    int phase1;
    int phase1offset;
    int phase2;
    int phase2offset;
    char fontsize;
    const char *name()
    {
        switch (icon)
        {
            case WeatherIcon::Cloud:
                return "cloud";
            case WeatherIcon::CloudWithLightRain:
                return "cloudlightrain";
            case WeatherIcon::CloudWithHeavyRain:
                return "cloudheavyrain";
            case WeatherIcon::CloudWithLightning:
                return "cloudlightning";
            case WeatherIcon::CloudWithHail:
                return "cloudhail";
            case WeatherIcon::SunWithRays:
                return "sun-with-rays";
            case WeatherIcon::SunWithoutRays:
                return "sun-no-rays";
            case WeatherIcon::Moon:
                return "moon";
            case WeatherIcon::Fog:
                return "fog";
            case WeatherIcon::SnowFlakes:
                return "snow";
            case WeatherIcon::Stars:
                return "stars";
            default:
                return "???";
        }
    };
};

template<class T>
class optional
{
private:
    bool _valid = false;
    T _value;
    
public:
    void set(const T &value) { _value = value; _valid = true; }
    void clear() { _value = T(); _valid = false; }
    bool isValid() const { return _valid; }
    const T &value() const { return _value; }
};

struct EnvironmentValues
{
    std::string invalidReason;
    optional<std::string> location;
    optional<tm> sunset;
    optional<tm> sunrise;
    optional<float> temperature;
    optional<float> windchill;
    optional<std::vector<WeatherLayer>> weather;
    optional<float> windangle;
    optional<float> windspeed;
    optional<float> airpressure;

    EnvironmentValues() { clear(); }
    ~EnvironmentValues() {}

    void clear()
    {
        invalidReason = "";
        location.clear();
        sunset.clear();
        sunrise.clear();
        temperature.clear();
        windchill.clear();
        weather.clear();
        windangle.clear();
        windspeed.clear();
        airpressure.clear();
    }

    void print()
    {
        printf("weather: ");
        if (invalidReason != "")
        {
            printf("error=%s\n", invalidReason.c_str());
        }
        else
        {
            if (location.isValid())
                printf("location=%s ", location.value().c_str());
            if (temperature.isValid())
                printf("temp=%.1f ", temperature.value());
            if (windchill.isValid())
                printf("gtemp=%.1f ", windchill.value());
            if (windspeed.isValid())
                printf("windspeed=%.1f ", windspeed.value());
            if (windangle.isValid())
                printf("windangle=%.2f ", windangle.value() * 180 /std::numbers::pi);
            if (airpressure.isValid())
                printf("airpressure=%.1f ", airpressure.value());
            if (weather.isValid())
            {
                printf("icons={");
                auto layers = weather.value();
                for (auto i=0; i<layers.size(); i++)
                {
                    if (i > 0) printf(",");
                    printf("%s", layers[i].name());
                }
                printf("} ");
            }
            if (sunrise.isValid())
                printf("up=%02d:%02d ", sunrise.value().tm_hour, sunrise.value().tm_min);
            if (sunset.isValid())
                printf("set=%02d:%02d ", sunset.value().tm_hour, sunset.value().tm_min);
            printf("\n");
        }
    }
};

class IEnvironment
{
public:
    virtual ~IEnvironment();

    virtual const char *source() const = 0;
    bool valid() const { return invalidReason().size() == 0; }

    /// @brief trigger a background update
    virtual void triggerUpdate() = 0;
    virtual bool isUpdating() const = 0;
    virtual timeinfo lastUpdate() const = 0;

    virtual std::string invalidReason() const = 0;
    virtual optional<std::string> location() const = 0;
    virtual optional<tm> &sunset() = 0;
    virtual optional<tm> &sunrise() = 0;
    virtual optional<float> temperature() const = 0;      // degrees
    virtual optional<float> windchill() const = 0;        // degrees
    virtual optional<std::vector<WeatherLayer>> weather() const = 0;
    virtual optional<float> windangle() const = 0;        // radians
    virtual optional<float> windspeed() const = 0;        // ms/s
    virtual optional<float> airpressure() const = 0;      // mbar
};

class EnvironmentBase : public IEnvironment
{
private:
    Event *_updateEvent;
    std::string _source;

public:
    EnvironmentBase(const char *src, Event *updateEvent) 
        : _updateEvent(updateEvent), _source(src) {}

    const char *source() const { return _source.c_str(); }

    void triggerUpdate() { triggerUpdateEvent()->set(); }
    virtual void update() = 0;
    Event *triggerUpdateEvent() { return _updateEvent; }
};

#endif
