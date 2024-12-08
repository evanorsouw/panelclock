
#ifndef I2CWRAPPER_H_
#define I2CWRAPPER_H_

#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c.h"

#define LOGGING

#if defined(LOGGING)
  #define LOG(...)
#else
  #define LOG(...) printf(__VA_ARGS__)
#endif

class I2CWrapper
{
private:
    i2c_port_t _port;
    gpio_num_t _clk;
    gpio_num_t _sda;
    int _timeout;
    int _speed;

public:
    I2CWrapper(i2c_port_t port, gpio_num_t clk, gpio_num_t sda, int timeout_ms = 100)
    {
        _port = port;
        _clk = clk;
        _sda = sda;
        _speed = 100000;
        _timeout = timeout_ms;
    }

    void start()
    {
        i2c_config_t config = {};
        config.mode = I2C_MODE_MASTER;
        config.sda_io_num = _sda;
        config.scl_io_num = _clk;
        config.sda_pullup_en = true; 
        config.scl_pullup_en = true;
        config.master.clk_speed = _speed;

        ESP_ERROR_CHECK(i2c_param_config(_port, &config));
        ESP_ERROR_CHECK(i2c_driver_install(_port, config.mode, 0, 0, 0));

        printf("intialized I2C port=%d as master, sda=%d, scl=%d, speed=%d\n", _port, _sda, _clk, _speed);
    }

    void end()
    {
        ESP_ERROR_CHECK(i2c_driver_delete(_port));
        printf("'terminated' I2C port=%d\n", _port);
    }

    void read(uint8_t slaveaddr, uint8_t reg, uint8_t *data, int len)
    {
        LOG("i2c.read(0x%02X,0x%02X,%p,%d) => [", slaveaddr, reg, data, len);
        i2c_master_write_read_device(_port, slaveaddr, &reg, 1, data, len, _timeout);
#ifdef LOGGING
        for (auto i=0; i<len; ++i)
        {
            LOG("0x%02x,", data[i]);
        }
        LOG("])\n");
#endif
    }

    void write(uint8_t slaveaddr, uint8_t reg, uint8_t data)
    {
        uint8_t buf[2] = { reg, data };
        LOG("i2c.write(0x%02X,0x%02X,0x%02X)\n", slaveaddr, reg, data);
        i2c_master_write_to_device(_port, slaveaddr, buf, 2, _timeout);
    }

    void write(uint8_t slaveaddr, uint8_t reg, uint8_t *data, uint8_t len)
    {       
        uint8_t buf[128];
        buf[0] = reg;
        memcpy(buf + 1, data, len);
        LOG("i2c.write(0x%02X,0x%02X,[", slaveaddr, reg);
#ifdef LOGGING
        for (auto i=0; i<len; ++i)
        {
            LOG("0x%02x,", data[i]);
        }
        LOG("])\n");
#endif
        i2c_master_write_to_device(_port, slaveaddr, buf, len + 1, _timeout);
    }
};

#endif