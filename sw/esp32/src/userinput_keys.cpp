
#include "userinput_keys.h"

UserInputKeys::UserInputKeys(gpio_num_t set, gpio_num_t up, gpio_num_t down, System &system)
    : _system(system)
{
    _ioSet = set;
    _ioUp = up;
    _ioDown = down;

    _keys.push_back(_keyinfo { .io=set, .key=KEY_SET, .pressedSince = 0 });
    _keys.push_back(_keyinfo { .io=up, .key=KEY_UP, .pressedSince = 0 });
    _keys.push_back(_keyinfo { .io=down, .key=KEY_DOWN, .pressedSince = 0 });

    std::for_each(_keys.begin(), _keys.end(), [](_keyinfo info){ gpio_set_direction(info.io, GPIO_MODE_INPUT);});
}

void UserInputKeys::updateTask()
{
    std::for_each(_keys.begin(), _keys.end(), [this](_keyinfo &info){ monitorIO(info);});
    vTaskDelay(20 / portTICK_PERIOD_MS);
}

int UserInputKeys::pendingKeys() const 
{
     return _keyQueue.size(); 
}

void UserInputKeys::flush()
{
    std::lock_guard lock(_mutex);
    while (_keyQueue.size() > 0)
    {
        _keyQueue.pop();
    }    
}

uint64_t UserInputKeys::howLongIsKeyDown(int key) const
{
    auto it = std::find_if(_keys.begin(), _keys.end(), [=](const _keyinfo &info) { return info.key == key; });
    if (it == _keys.end() || it->pressedSince == 0)
        return 0;
    auto elapsed = (_system.now().msticks() - it->pressedSince);
    return elapsed;
}

KeyPress UserInputKeys::getKey() 
{
    std::lock_guard lock(_mutex);
    if (pendingKeys() == 0)
        return KeyPress();
        
    auto key = _keyQueue.front();
    _keyQueue.pop();
    return key;
}

bool UserInputKeys::hasKeyDown(int key, int ms)
{
    auto it = std::find_if(_keys.begin(), _keys.end(), [=](const _keyinfo &info) { return info.key == key; });
    if (it == _keys.end() || it->pressedSince == 0)
        return false;
    auto elapsed = (_system.now().msticks() - it->pressedSince);
    if (elapsed < ms)
        return false;
    it->ignore = true;
    return true;
}

void UserInputKeys::monitorIO(_keyinfo &info)
{
    auto pressed = !gpio_get_level(info.io);
    if (pressed != (info.pressedSince != 0))
    {
        if (pressed)
        {
            info.pressedSince = _system.now().msticks();
        }
        else
        {
            if (info.ignore)
            {
                info.ignore = false;
            }
            else
            {
                auto elapsed = _system.now().msticks() - info.pressedSince;
                std::lock_guard lock(_mutex);
                _keyQueue.push(KeyPress(info.key, elapsed));
                printf("keypress: %d (%lldms)\n", info.key, elapsed);
            }
            info.pressedSince = 0;
        }
    }
}

