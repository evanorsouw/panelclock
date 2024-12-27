
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <string>
#include <time.h>
#include "timeinfo.h"

enum class weathertype {
    unknown,
    clouded,      // 1:bewolkt    
    lightning,    // 2:bliksem    
    showers,      // 3:buien    
    hail,         // 4:hagel    
    partlycloudy, // 5:halfbewolkt    
    cloudyrain,   // 6:halfbewolkt_regen    
    clearnight,   // 7:helderenacht    
    cloudy,       // 8:lichtbewolkt    
    fog,          // 9:mist    
    cloudednight, // 10:nachtbewolkt    
    nightfog,     // 11:nachtmist
    rain,         // 12:regen    
    snow,         // 13:sneeuw
    sunny,        // 14:zonnig    
    heavyclouds,  // 15:storm
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
    optional<weathertype> weather;
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
};

struct Environment
{
    virtual ~Environment() {}

    virtual bool valid() { return invalidReason().size() == 0; }
    virtual timeinfo lastupdate() const = 0;

    /// @brief updates the internal representation 
    /// @return the preferred number of ms before making a new attempt to update the info.
    virtual int update() = 0;

    virtual bool isupdating() const = 0;
    virtual std::string invalidReason() const = 0;
    virtual optional<std::string> location() const = 0;
    virtual optional<tm> sunset() const = 0;
    virtual optional<tm> sunrise() const = 0;
    virtual optional<float> temperature() const = 0;      // degrees
    virtual optional<float> windchill() const = 0;        // degrees
    virtual optional<weathertype> weather() const = 0;
    virtual optional<float> windangle() const = 0;        // radians
    virtual optional<float> windspeed() const = 0;        // ms/s
    virtual optional<float> airpressure() const = 0;      // mbar

    virtual void _setweathertype(int type) = 0;
};

#endif
