
#ifndef _RENDERBASE_H_
#define _RENDERBASE_H_

#include "bitmap.h"
#include "environment.h"
#include "graphics.h"
#include "applicationcontext.h"
#include "system.h"
#include "userinput.h"

class RenderBase
{
protected:
    ApplicationContext &_appctx;
    Graphics &_graphics;
    Environment &_environment;
    System &_system;
    UserInput &_userinput;

public:
    RenderBase(ApplicationContext &appdata, Graphics &graphics, Environment &env, System &sys, UserInput &userinput) 
        : _appctx(appdata)
        , _graphics(graphics)
        , _environment(env) 
        , _system(sys)
        , _userinput(userinput)
    {
    }
    virtual ~RenderBase() {}

    bool timeout(uint64_t timer, int timeoutMs) { return _appctx.timeout(timer, timeoutMs); }
    void starttimer(uint64_t &timer) { _appctx.starttimer(timer); }
    int elapsed(uint64_t timer) { return _appctx.elapsed(timer); }
    long drawtime() { return _appctx.msSinceMidnight(); }    
    float phase(int cycleMs, bool wave, int offsetMs = 0) { return _appctx.phase(cycleMs, wave, offsetMs); }

    void graduallyUpdateVariable(float &current, float target, float speed);
    void graduallyUpdateVariable(float &current, float targetLo, float targetHi, float speed);

    virtual void init() {}
    virtual void render(Bitmap &screen) {}
    virtual bool interact() { return false; }
};

#endif