
#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <algorithm>
#include <functional>

#include "bitmap.h"
#include "graphics.h"

class Animation
{
private:
    float _start;
    float _end;

protected:
    Graphics &_graphics;

public:
    /// @brief add animation cycle with an aboslute start-end timepoint.
    /// @param start the time to start the animation relative from boot
    /// @param end the time to end the animation relative from boot
    /// @param handler the handler that is given a number where [0.0, 1.0] indicate
    /// in between start/end, <0 indicates animation not yet started, >1 indicates

    /// animation passed endtime.
    Animation(Graphics &graphics, float start, float end)
        : _start(start)
        , _end(end) 
        , _graphics(graphics) {}

    virtual ~Animation() {}

    /// @brief  draw animation for given moment in time
    /// @param when relative moment in time.
    /// @return true when done (passed end-point);
    bool run(Bitmap &screen, float when)
    {
        auto fraction = (when - _start) / (_end - _start);
        animate(screen, fraction);
        return !(when > _end);
    }

protected:
    virtual void animate(Bitmap &screen, float when) {}
};

#endif