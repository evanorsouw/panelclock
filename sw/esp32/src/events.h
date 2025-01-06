#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

class Events;

class Event
{
friend Events;
private:
    EventGroupHandle_t _eventGroup;
    EventBits_t _eventBit;
    std::string _name;

    Event(EventGroupHandle_t eventGroup, EventBits_t bit, const char *name)
    {
        _eventGroup = eventGroup;
        _eventBit = bit;
        _name = name;
    }

public:
    const char *name() const { return _name.c_str(); }
    void set() 
    { 
        xEventGroupSetBits(_eventGroup, _eventBit); 
    }
    EventBits_t bit() const { return _eventBit; }
    bool wasSet() { return wait(0); }
    bool wait(int timeoutMs)
    {
        auto ticks = timeoutMs / portTICK_PERIOD_MS;
        return xEventGroupWaitBits(_eventGroup, _eventBit, true, false, ticks) == _eventBit;
    }
};

class Events
{
private:
    EventGroupHandle_t _eventGroup;
    uint32_t _nextBit;
    uint32_t _allocatedBits;
    std::vector<Event *> _events;

public:
    Events()
    {
        _eventGroup = xEventGroupCreate();
        _nextBit = 1;
        _allocatedBits = 0;
    }

    Event *allocate(const char *name)
    {
        auto bit = _nextBit;
        _allocatedBits |= bit;
        _nextBit <<= 1;
        auto event = new Event(_eventGroup, bit, name);
        _events.push_back(event);
        return event;
    }

    EventBits_t wait(uint32_t delayInMs=0)
    {
        auto ticks = delayInMs == 0 ? portMAX_DELAY : delayInMs / portTICK_PERIOD_MS;
        auto bits = xEventGroupWaitBits(_eventGroup, _allocatedBits, false, false, ticks);
        return bits;
    }
};

#endif