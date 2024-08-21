
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <string>
#include <time.h>

enum class weathertype {
    unknown,
    // bewolkt
    clouded,
    // bliksem
    lightning,
    // buien
    showers,
    // hagel
    hail,
    // halfbewolkt
    partlycloudy,
    // halfbewolkt_regen
    cloudyrain,
    // helderenacht
    clearnight,
    // lichtbewolkt
    cloudy,
    // mist
    fog,
    // nachtbewolkt
    cloudednight,
    // nachtmist
    nightfog,
    // regen
    rain,
    // sneeuw
    snow,
    // zonnig
    sunny,
    // zwaarbewolkt
    heavyclouds,
};

struct Environment
{
    virtual ~Environment() {}

    virtual void update() = 0;

    virtual bool valid() const = 0; 
    virtual tm sunset() const = 0;
    virtual tm sunrise() const = 0;
    virtual float temperature() const = 0;
    virtual float windchill() const = 0;
    virtual weathertype weather() const = 0;
    virtual float windangle() const = 0;
    virtual float windspeed() const = 0;
    virtual float airpressure() const = 0;
};

#endif
