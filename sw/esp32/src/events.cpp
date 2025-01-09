#include "events.h"

#if 1
  #define LOG(...) printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif

Event::Event(EventGroupHandle_t eventGroup, EventBits_t bit, const char *name)
{
    _eventGroup = eventGroup;
    _eventBit = bit;
    _name = name;
}

void Event::set() 
{ 
    LOG("trigger event %s\n", name());
    xEventGroupSetBits(_eventGroup, _eventBit); 
}

bool Event::wasSet() 
{ 
    return wait(0);
}

bool Event::wait(int timeoutMs)
{
    auto ticks = timeoutMs / portTICK_PERIOD_MS;
    auto bits = xEventGroupWaitBits(_eventGroup, _eventBit, true, false, ticks);
    return (bits & _eventBit) != 0;
}

Events::Events()
{
    _eventGroup = xEventGroupCreate();
    _nextBit = 1;
    _allocatedBits = 0;
}

Event *Events::allocate(const char *name)
{
    auto bit = _nextBit;
    _allocatedBits |= bit;
    _nextBit <<= 1;
    auto event = new Event(_eventGroup, bit, name);
    _events.push_back(event);
    return event;
}

EventBits_t Events::wait(uint32_t delayInMs)
{
    auto ticks = delayInMs == 0 ? portMAX_DELAY : delayInMs / portTICK_PERIOD_MS;
    auto bits = xEventGroupWaitBits(_eventGroup, _allocatedBits, false, false, ticks);
    return bits;
}
