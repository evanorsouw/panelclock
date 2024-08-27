
#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <functional>

class Animation
{
private:
    float _start;
    float _end;
    std::function<void(float)> _handler;
    
public:
    /// @brief add animation cycle with an aboslute start-end timepoint.
    /// @param start the time to start the animation relative from boot
    /// @param end the time to end the animation relative from boot
    /// @param handler the handler that is given a number where [0.0, 1.0] indicate
    /// in between start/end, <0 indicates animation not yet started, >1 indicates
    /// animation passed endtime.
    Animation(float start, float end, std::function<void(float)> handler)
        : _start(start)
        , _end(end)
        , _handler(handler) {}

    /// @brief  draw animation for given moment in time
    /// @param when relative moment in time.
    /// @return true when done (passed end-point);
    bool run(float when)
    {
        _handler((when - _start) / (_end - _start));
        return !(when > _end);
    }
};

#endif