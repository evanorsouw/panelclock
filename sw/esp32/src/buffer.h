
#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <algorithm>
#include <cstdint>
#include <cstring>

class Buffer
{
private:
    uint8_t *_buffer;
    int _capacity;
    int _size;

public:
    Buffer(int initialCapacity = 32)
    {
        _capacity = 0;
        _size = 0;
        _buffer = nullptr;
        assureRoomFor(initialCapacity);
    }
    virtual ~Buffer()
    {
        delete[] _buffer;
        _buffer = nullptr;
    }

    int capacity() const { return _capacity; }
    int size() const { return _size; }
    uint8_t *data() const { return _buffer; }

    void clear() { _size = 0; }

    void set(const char *string, bool includeTerminator = true)
    {
        clear();
        auto len = strlen(string) + (includeTerminator ? 1 : 0);
        assureRoomFor(len);
        add((uint8_t *)string, len);
    }

    void copyfrom(Buffer &rhs) 
    { 
        clear();
        assureRoomFor(rhs.size());
        add(rhs.data(), rhs.size());
    }

    void add(uint8_t *data, int len)
    {
        assureRoomFor(len);
        std::memcpy(_buffer + _size, data, len);
        _size += len;
    }

    void add(uint8_t byte)
    {
        assureRoomFor(1);
        _buffer[_size++] = byte;
    }

    void assureRoomFor(int additionalBytes)
    {
        auto requiredCapacity = _size + additionalBytes;
        if (requiredCapacity <= _capacity)
            return;

        auto newcapacity = std::max(_capacity * 2, requiredCapacity);
        auto newbuffer = new uint8_t[newcapacity];
        if (_size > 0)
        {
            std::memcpy(newbuffer, _buffer, _size);
        }
        delete[] _buffer;
        _buffer = newbuffer;
        _capacity = newcapacity;
    }
};

#endif
