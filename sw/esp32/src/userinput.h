
#ifndef _USERINPUT_H_
#define _USERINPUT_H_

struct KeyPress
{
    KeyPress() : key(0), presstime(0) {}
    KeyPress(int key, uint64_t presstime) : key(key), presstime(presstime) {}
    int key;
    uint64_t presstime;
};

class UserInput
{
public:
    static const int KEY_SET  = 0x101;
    static const int KEY_UP   = 0x102;
    static const int KEY_DOWN = 0x103;
    static const int KEY_BOOT = 0x104;

    virtual ~UserInput() {}

    /// @brief check if any unretrieved keypresses were recorded
    /// @return number of pending keypresses or 0 if none.
    virtual int pendingKeys() const = 0;

    virtual void flush() = 0;

    /// @brief check for how long a key is currently pressed
    /// @param key the key to check.
    /// @return number of ms that key has been pressed (0 if not pressed)
    virtual uint64_t howLongIsKeyDown(int key) const = 0;

    /// @brief check if a key has been pressed for a minimum amount of time.
    /// if the key was pressed for that long, it will no longer be recorded as key-press.
    /// @param key the key to check.
    /// @return true if key was down for the requested period
    virtual bool hasKeyDown(int key, int ms) = 0;

    /// @brief get a key from the queue.
    /// @return object indicating a key was pushed and for how long.
    virtual KeyPress getKeyPress() = 0;
};

#endif

