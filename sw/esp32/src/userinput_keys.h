
#ifndef _USERINPUT_KEYS_H_
#define _USERINPUT_KEYS_H_

#include <mutex>
#include <queue>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include "system.h"
#include "userinput.h"

class UserInputKeys : public UserInput
{
private:
    struct _keyinfo {
        gpio_num_t io;
        int key;
        bool lastPressed;
        uint64_t pressedSince;
        bool ignore;
    };

private:
    gpio_num_t _ioSet;
    gpio_num_t _ioUp;
    gpio_num_t _ioDown;
    gpio_num_t _ioBoot;
    System &_system;
    std::vector<_keyinfo> _keys;
    std::mutex _mutex;
    std::queue<KeyPress> _keyQueue;
    
public:
    UserInputKeys(gpio_num_t set, gpio_num_t up, gpio_num_t down, gpio_num_t boot, System &system);

    void updateTask();

    int pendingKeys() const;
    void flush();
    KeyPress getKeyPress();
    bool hasKeyDown(int key, int ms);
    uint64_t howLongIsKeyDown(int key) const;

private:
    void monitorIO(_keyinfo &info);
    int flipKey(int key) const;
};

#endif

