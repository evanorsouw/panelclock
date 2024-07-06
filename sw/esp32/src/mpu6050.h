
#ifndef _MPU6050_H_
#define _MPU6050_H_

#include "math.h"
#include "i2cwrapper.h"

class MPU6050
{
private:
    const int SLAVE_ADDR = 0x69;
    const int PWR_MGMT_1 = 0x6B;
    I2CWrapper *_i2c;

public:
    MPU6050(I2CWrapper *i2c)
    {
        _i2c = i2c;
    }

    virtual ~MPU6050() {}

    void start()
    {
        // wake up the device
        _i2c->write(SLAVE_ADDR, PWR_MGMT_1, 0);
    }

    void end() {}

    void readAngles()
    {
        uint8_t buf[14] = {0};
        _i2c->read(SLAVE_ADDR, 59, buf, sizeof(buf));

        printf("data: ");
        for (int i=0; i<sizeof(buf); ++i)
        {
            printf("%02x ", buf[i]);
        }
        printf("\n");

        auto rawx = getRaw(buf[0], buf[1]);
        auto rawy = getRaw(buf[2], buf[3]);
        auto rawz = getRaw(buf[4], buf[5]);
        auto rawtmp = getRaw(buf[6], buf[7]);

        auto accx = rawx /16384.0f;

        
        auto accy = rawy /16384.0f;
        auto accz = rawz /16384.0f;
        auto temp = rawtmp / 340.0f + 36.53f;

        auto roll = atan2(accy, accx) * 180 / 3.141592653f;
        auto pitch = atan2(-accx, sqrt(accy * accy + accz * accz)) * 180 / 3.141592653f;

        printf("accx=%f, accy=%f, accz=%f, roll=%.1f, pitch=%.1f, temp=%.1f\n", accx, accy, accz, roll, pitch, temp);
    }

private:
    int getRaw(uint8_t high, uint8_t low)
    {
        auto value = (high << 8) | low;
        if (value >= 0x8000)
        {
            value = -(65536 - value);
        }
        return value;
    }
};

#endif
