
#ifndef _STUSB4500_H_
#define _STUSB4500_H_

#include "i2cwrapper.h"

class STUSB4500
{
private:
    const int SLAVE_ADDR = 0x28;
    const int ALERT_STATUS_1_MASK = 0x0C;
    I2CWrapper &_i2c;
    uint8_t _sectors[5][8];

public:
    struct pdoconfig
    {
        float voltage;
        int voltageLowerMargin;
        int voltageUpperMargin;
        float maxCurent;        
    };

public:
    STUSB4500(I2CWrapper &i2c)
        : _i2c(i2c) {}

    bool init();

    uint8_t getPdoNumber();
    void setPdoNumber(uint8_t value);
    void assurePdoNumber(int pdo);
    pdoconfig getPdo(uint8_t pdo);
    void setPdo(uint8_t pdo, pdoconfig& config);
    void setPdo(uint8_t pdo, float voltage, float maxCurrent);
    void assurePdo(int pdo, float voltage, float current);
    void setDefaults();

private:
    void enterNVM();
    void leaveNVM();
    void waitnvmready();
    void readsector(uint8_t sector, uint8_t *buf);
    void writesector(uint8_t sector, uint8_t *buf);
    uint8_t currentF2I(float current);
    float currentI2F(uint8_t current);
    uint16_t voltageF2I(float voltage);
    float voltageI2F(uint16_t voltage);
    uint8_t voltageMarginP2I(int percentage);
    int voltageMarginI2P(uint8_t storevalue);
};

#endif
