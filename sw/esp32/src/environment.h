
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <string>
#include <time.h>

enum class weathertype {
    unknown,
    clouded,      // bewolkt    
    lightning,    // bliksem    
    showers,      // buien    
    hail,         // hagel    
    partlycloudy, // halfbewolkt    
    cloudyrain,   // halfbewolkt_regen    
    clearnight,   // helderenacht    
    cloudy,       // lichtbewolkt    
    fog,          // mist    
    cloudednight, // nachtbewolkt    
    nightfog,     // nachtmist
    rain,         // regen    
    snow,         // regen    
    sunny,        // zonnig    
    heavyclouds,  // zonnig
};

template<class T>
class optional
{
private:
    bool _valid = false;
    T _value;
    
public:
    void set(const T &value) { _value = value; _valid = true; }
    void clear() { _valid = false; }
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
};

#endif
