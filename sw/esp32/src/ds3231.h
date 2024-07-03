
#ifndef _DS3231_H_
#define _DS3231_H_

#include "i2cwrapper.h"

struct tod {
    int year;
    uint8_t mon;
    uint8_t mday;
    uint8_t wday;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};

class DS3231
{
private:
    const int SLAVE_ADDR = 0x68;
    I2CWrapper *_i2c;
    tod _tod1;
    tod _tod2;
    tod *_lastReadTod;

public:
    DS3231(I2CWrapper *i2c)
    {
        _i2c = i2c;
        _lastReadTod = &_tod1;
    }

    virtual ~DS3231() {}

    void readTimeFromChip()
    {
        uint8_t buf[7] = {0};
        _i2c->read(SLAVE_ADDR, 0, buf, sizeof(buf));

        _lastReadTod = extractTime(_lastReadTod == &_tod1 ? &_tod2 : &_tod1, buf);

        printf("time: ");
        for (int i=0; i<sizeof(buf); ++i)
        {
            printf("%02x ", buf[i]);
        }
        printf(" => %04d-%02d-%02d %02d:%02d:%02d\n", 
            _lastReadTod->year,
            _lastReadTod->mon,
            _lastReadTod->mday,
            _lastReadTod->hour,
            _lastReadTod->min,
            _lastReadTod->sec);
    }

    tod *getTime() const { return _lastReadTod; }

private: 
    tod *extractTime(tod *tod, uint8_t *buf)
    {
        tod->sec = bcd2dec(buf[0]);
        tod->min = bcd2dec(buf[1]);
        tod->hour = bcd2dec(buf[2]);
        tod->wday = bcd2dec(buf[3]);
        tod->mday = bcd2dec(buf[4]);
        tod->mon = bcd2dec(buf[5] & 0x7F);
        tod->year = 1900 + bcd2dec(buf[6]) + ((buf[5] & 0x80) ? 100 : 0);
        return tod;
    }

     uint8_t bcd2dec(uint8_t bcd)
     {
        return (bcd >> 4) * 10 + (bcd & 0xF);
     }
};

#endif
