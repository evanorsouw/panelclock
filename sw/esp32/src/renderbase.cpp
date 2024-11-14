
#include <algorithm>
#include <cmath>

#include "renderbase.h"

void RenderBase::graduallyUpdateVariable(float &current, float target, float speed)
{
    auto diff = std::abs(current - target);
    if (current != target)
    {
        auto step = std::min(diff, std::log(diff + 1) * speed);
        current += current < target ? step : -step;
    }
}

void RenderBase::graduallyUpdateVariable(float &current, float targetLo, float targetHi, float speed)
{
    float target = current;
    if (current < targetLo)
    {        
        target = targetLo;
    }
    else if (current > targetHi)
    {
        target = targetHi;
    }
    graduallyUpdateVariable(current, target, speed);
}


