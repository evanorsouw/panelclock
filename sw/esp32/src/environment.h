
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

struct Environment
{
    virtual ~Environment() {}

    virtual bool valid() const = 0; 
    virtual tm sunset() const = 0;
    virtual tm sunrise() const = 0;
    virtual float temperature() const = 0;      // degrees
    virtual float windchill() const = 0;        // degrees
    virtual weathertype weather() const = 0;
    virtual float windangle() const = 0;        // radians
    virtual float windspeed() const = 0;        // ms/s
    virtual float airpressure() const = 0;      // mbar
};

#endif
