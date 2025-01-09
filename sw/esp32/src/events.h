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

    Event(EventGroupHandle_t eventGroup, EventBits_t bit, const char *name);

public:
    const char *name() const { return _name.c_str(); }
    EventBits_t bit() const { return _eventBit; }
    void set();
    bool wasSet();
    bool wait(int timeoutMs);
};

class Events
{
private:
    EventGroupHandle_t _eventGroup;
    uint32_t _nextBit;
    uint32_t _allocatedBits;
    std::vector<Event *> _events;

public:
    Events();

    Event *allocate(const char *name);
    EventBits_t wait(uint32_t delayInMs=0);
};

#endif