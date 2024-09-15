
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
    optional(){}
    optional(const optional &rhs) : _valid(rhs._valid), _value(rhs._value) {}
    void set(const T &value) { _value = value; _valid = true; }
    void clear() { _valid = false; }
    bool isValid() const { return _valid; }
    const T &value() const { return _value; }
};

struct EnvironmentValues
{
    bool valid;
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
        valid = false;
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

    virtual bool valid() const = 0;
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
