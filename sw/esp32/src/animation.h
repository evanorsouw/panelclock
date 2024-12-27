
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

public:
    /// @brief add animation cycle with an aboslute start-end timepoint.
    /// @param start the time to start the animation relative from boot
    /// @param end the time to end the animation relative from boot
    Animation(float start, float end)
        : _start(start)
        , _end(end) {}

    virtual ~Animation() {}

    /// @brief  draw animation for given moment in time
    /// @param when relative moment in time.
    /// @return true when done (passed end-point);
    bool run(Graphics& graphics, float when)
    {
        auto fraction = (when - _start) / (_end - _start);
        animate(graphics, fraction);
        return !(when > _end);
    }

protected:
    virtual void animate(Graphics& graphics, float when) {}
};

#endif