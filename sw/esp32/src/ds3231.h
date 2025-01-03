
#ifndef _DS3231_H_
#define _DS3231_H_

#include "i2cwrapper.h"

struct tod {
    int year;       // 1900..2099
    uint8_t mon;    // 0..11
    uint8_t mday;   // 1..31
    uint8_t wday;   // 0..6
    uint8_t hour;   // 0..23
    uint8_t min;    // 0..59
    uint8_t sec;    // 0..59
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
        _tod1 = {0};
        _tod2 = {0};
        _lastReadTod = &_tod1;
    }

    virtual ~DS3231() {}

    void readTimeFromChip()
    {
        uint8_t buf[7] = {0};
        _i2c->read(SLAVE_ADDR, 0, buf, sizeof(buf));

        _lastReadTod = extractTime(_lastReadTod == &_tod1 ? &_tod2 : &_tod1, buf);

        printf("rtc read time: ");
        for (int i=0; i<sizeof(buf); ++i)
        {
            printf("%02x ", buf[i]);
        }
        printf("rtc read time => ");
        printTod(_lastReadTod);
    }

    void setTime(tod *now)
    {
        printf("rtc write time ");
        printTod(now);        

        _i2c->write(SLAVE_ADDR, 0, dec2bcd(now->sec));
        _i2c->write(SLAVE_ADDR, 1, dec2bcd(now->min));
        _i2c->write(SLAVE_ADDR, 2, dec2bcd(now->hour));
        _i2c->write(SLAVE_ADDR, 3, dec2bcd(now->wday + 1));
        _i2c->write(SLAVE_ADDR, 4, dec2bcd(now->mday));
        _i2c->write(SLAVE_ADDR, 5, dec2bcd(now->mon + 1) | ((now->year >= 2000) ? 0x80 : 0x00));
        _i2c->write(SLAVE_ADDR, 6, dec2bcd(now->year % 100));
    }

    tod *getTime() const { return _lastReadTod; }

private: 
    tod *extractTime(tod *tod, uint8_t *buf)
    {
        tod->sec = bcd2dec(buf[0]);
        tod->min = bcd2dec(buf[1]);
        tod->hour = bcd2dec(buf[2]);
        tod->wday = bcd2dec(buf[3]) - 1;
        tod->mday = bcd2dec(buf[4]);
        tod->mon = bcd2dec(buf[5] & 0x7F) - 1;
        tod->year = 1900 + bcd2dec(buf[6]) + ((buf[5] & 0x80) ? 100 : 0);
        return tod;
    }

     uint8_t bcd2dec(uint8_t bcd)
     {
        return (bcd >> 4) * 10 + (bcd & 0xF);
     }
     
     uint8_t dec2bcd(uint8_t dec)
     {
        return ((dec / 10) << 4) | (dec % 10);
     }

    void printTod(tod *now)
    {
        printf("%04d-%02d-%02d %02d:%02d:%02d (wday:%d)\n", 
            now->year, now->mon + 1, now->mday,
            now->hour, now->min, now->sec,
            now->wday);
    }
};

#endif
