
#include <algorithm>
#include "stusb4500.h"

#define DEFAULT                0xFF

#define FTP_CUST_PASSWORD_REG  0x95
#define FTP_CUST_PASSWORD      0x47

#define FTP_CTRL_0             0x96
#define FTP_CTRL_0_CUST_PWR    0x80 
#define FTP_CTRL_0_CUST_RST_ACTIVE  0x00
#define FTP_CTRL_0_CUST_RST_INACTIVE  0x40
#define FTP_CTRL_0_CUST_REQ    0x10
#define FTP_CTRL_0_CUST_SECT_MSK   0x07
#define FTP_CTRL_0_CUST_SECTOR(x)   ((x)&0x07)
#define FTP_CTRL_1             0x97
#define FTP_CTRL_1_CUST_SER_MSK    0xF8
#define FTP_CTRL_1_CUST_SER_SECTOR(x)  (0x08 << ((x)&0x07))
#define FTP_CTRL_1_CUST_SER_SECTOR0    0x08
#define FTP_CTRL_1_CUST_SER_SECTOR1    0x10
#define FTP_CTRL_1_CUST_SER_SECTOR2    0x20
#define FTP_CTRL_1_CUST_SER_SECTOR3    0x40
#define FTP_CTRL_1_CUST_SER_SECTOR4    0x80
#define FTP_CTRL_1_CUST_OPCODE_MSK 0x07
#define FTP_CTRL_1_CUST_OPCODE_READ 0x00
#define FTP_CTRL_1_CUST_OPCODE_SHIFT_IN_DATA_ON_PROGRAM_LOAD 0x01
#define FTP_CTRL_1_CUST_OPCODE_SHIFT_IN_DATA_ON_SECTOR_ERASE 0x02
#define FTP_CTRL_1_CUST_OPCODE_ERASE 0x05
#define FTP_CTRL_1_CUST_OPCODE_PROGRAM 0x06
#define FTP_CTRL_1_CUST_OPCODE_SOFT_PROGRAM_ARRAY 0x07
#define RW_BUFFER              0x53
#define TX_HEADER_LOW          0x51
#define PD_COMMAND_CTRL        0x1A
#define DPM_PDO_NUMB           0x70


bool STUSB4500::init()
{
    uint8_t buf[20];

    _i2c.read(SLAVE_ADDR, 0x0D, buf, 10);
    _i2c.read(SLAVE_ADDR, 0x2F, buf + 10, 1);
    auto deviceId = buf[10];

    // disable all interupts
    _i2c.write(SLAVE_ADDR, ALERT_STATUS_1_MASK, 0xFF);
    
    printf("STUSB4500: device id=0x%02X\n", deviceId);

    return deviceId != 0; 
}

void STUSB4500::setDefaults()
{
    uint8_t default_sector[5][8] = 
    {
      {0x00,0x00,0xB0,0xAA,0x00,0x45,0x00,0x00},
      {0x10,0x40,0x9C,0x1C,0xFF,0x01,0x3C,0xDF},
      {0x02,0x40,0x0F,0x00,0x32,0x00,0xFC,0xF1},
      {0x00,0x19,0x56,0xAF,0xF5,0x35,0x5F,0x00},
      {0x00,0x4B,0x90,0x21,0x43,0x00,0x40,0xFB}
    };
    
    enterNVM();
    for (auto i=0; i<5; ++i)
    {
        writesector(i, default_sector[i]);
    }
    leaveNVM();
}

uint8_t STUSB4500::getPdoNumber()
{
    uint8_t bank[8];
    enterNVM();
    readsector(3, bank);
    leaveNVM();
    return (bank[2] >> 1) & 0x03;
}

void STUSB4500::assurePdo(int pdo, float voltage, float current)
{
    voltage = voltageI2F(voltageF2I(voltage));
    current = currentI2F(currentF2I(current));

    auto config = getPdo(pdo);
    if (config.voltage != voltage || config.maxCurent != current)
    {
        printf("PD: changed pdo%d from %.1fV/%.1fA to %.1fV/%.1fA\n", pdo, config.voltage, config.maxCurent, voltage, current);
        config.voltage = voltage;
        config.maxCurent = current;
        config.voltageLowerMargin = 15;
        config.voltageLowerMargin = 10;
        setPdo(pdo, config);
    }
}

void STUSB4500::assurePdoNumber(int pdo)
{
    pdo = pdo & 0x03;

    if (getPdoNumber() != pdo)
    {
        printf("PD: changed pdo from %d to %d\n", getPdoNumber(), pdo);
        setPdoNumber(pdo);
    }
}

void STUSB4500::setPdoNumber(uint8_t value)
{
    uint8_t buf[8];
    enterNVM();
    readsector(3, buf);
    buf[2] = (buf[2] & 0xF9) | ((value & 3) << 1);
    writesector(3, buf);
    leaveNVM();
}

STUSB4500::pdoconfig STUSB4500::getPdo(uint8_t pdo)
{
    uint8_t bank3[8], bank4[8];

    enterNVM();
    readsector(3, bank3);
    readsector(4, bank4);
    leaveNVM();

    pdoconfig config;

    switch (pdo)
    {
        case 1:
            config.voltage = 5;
            config.voltageLowerMargin = voltageMarginI2P(bank3[3] & 0x0F);
            config.voltageUpperMargin = voltageMarginI2P(bank3[3] >> 4);
            config.maxCurent = currentI2F(bank3[2] >> 4);
            break;
        case 2:
            config.voltage = voltageI2F((bank4[0] & 0x03) | (bank4[1]<<2));
            config.voltageLowerMargin = voltageMarginI2P(bank3[4] >> 4);
            config.voltageUpperMargin = voltageMarginI2P(bank3[5] & 0x0f);
            config.maxCurent = currentI2F(bank3[4] & 0x0F);
            break;
        case 3:
            config.voltage = voltageI2F(((bank4[3] & 0x03) << 8) | bank4[2]);
            config.voltageLowerMargin = voltageMarginI2P(bank3[6] & 0x0F);
            config.voltageUpperMargin = voltageMarginI2P(bank3[6] >> 4);
            config.maxCurent = currentI2F(bank3[5] >> 4);
            break;
    }
    return config;
}

void STUSB4500::setPdo(uint8_t pdo, pdoconfig& config)
{
    uint8_t bank3[8], bank4[8];

    enterNVM();
    readsector(3, bank3);
    readsector(4, bank4);

    switch (pdo)
    {
        case 1:
            bank3[3] = (bank3[3] & 0xF0) | voltageMarginP2I(config.voltageLowerMargin);
            bank3[3] = (bank3[3] & 0x0F) | (voltageMarginP2I(config.voltageUpperMargin) << 4);
            bank3[2] = (bank3[2] & 0x0F) | (currentF2I(config.maxCurent) << 4);
            break;
        case 2:
            bank4[0] = (bank4[0] & 0xFC) | (voltageF2I(config.voltage) & 0x03);
            bank4[1] = voltageF2I(config.voltage) >> 2;
            bank3[4] = (bank3[4] & 0x0F) | voltageMarginP2I(config.voltageLowerMargin) << 4;
            bank3[5] = (bank3[5] & 0xF0) | (voltageMarginP2I(config.voltageUpperMargin) & 0x0F);
            bank3[4] = (bank3[4] & 0xF0) | currentF2I(config.maxCurent);
            break;
        case 3:
            bank4[3] = (bank4[3] & 0xFC) | (voltageF2I(config.voltage) >> 8);
            bank4[2] = voltageF2I(config.voltage) & 0xFF;
            bank3[6] = (bank3[6] & 0xF0) | voltageMarginP2I(config.voltageLowerMargin);
            bank3[6] = (bank3[6] & 0x0F) | (voltageMarginP2I(config.voltageUpperMargin) << 4);
            bank3[5] = (bank3[5] & 0x0F) | currentF2I(config.maxCurent) << 4;
            break;
    }
    writesector(3, bank3);
    writesector(4, bank4);
    leaveNVM();
}

void STUSB4500::setPdo(uint8_t pdo, float voltage, float maxCurrent)
{
    pdoconfig config {
        .voltage = voltage,
        .voltageLowerMargin = 15,
        .voltageUpperMargin = 10,
        .maxCurent = maxCurrent
    };
    setPdo(pdo, config);
}

void STUSB4500::enterNVM()
{
    // gain access
    _i2c.write(SLAVE_ADDR, FTP_CUST_PASSWORD_REG, FTP_CUST_PASSWORD);

    _i2c.write(SLAVE_ADDR, 0x53, 0x00);
    // power-up (give reset pulse)
    _i2c.write(SLAVE_ADDR, FTP_CTRL_0, FTP_CTRL_0_CUST_RST_ACTIVE);
    vTaskDelay(1);
    _i2c.write(SLAVE_ADDR, FTP_CTRL_0, FTP_CTRL_0_CUST_RST_INACTIVE);
}

void STUSB4500::leaveNVM()
{
    _i2c.write(SLAVE_ADDR, FTP_CTRL_0, FTP_CTRL_0_CUST_RST_INACTIVE);
    _i2c.write(SLAVE_ADDR, FTP_CTRL_1, 0x00);
    _i2c.write(SLAVE_ADDR, FTP_CUST_PASSWORD_REG, 0x00);
}

void STUSB4500::readsector(uint8_t sector, uint8_t *buf8)
{
    _i2c.write(SLAVE_ADDR, FTP_CTRL_1, FTP_CTRL_1_CUST_OPCODE_READ);
    _i2c.write(SLAVE_ADDR, FTP_CTRL_0, FTP_CTRL_0_CUST_SECTOR(sector) | FTP_CTRL_0_CUST_RST_INACTIVE | FTP_CTRL_0_CUST_REQ);
    waitnvmready();
    _i2c.read(SLAVE_ADDR, 0x53, buf8, 8);
}

void STUSB4500::writesector(uint8_t sector, uint8_t *buf8)
{
    // erase current sector data from NVM
    _i2c.write(SLAVE_ADDR, FTP_CTRL_1, FTP_CTRL_1_CUST_SER_SECTOR(sector) | FTP_CTRL_1_CUST_OPCODE_SHIFT_IN_DATA_ON_SECTOR_ERASE);
    _i2c.write(SLAVE_ADDR, FTP_CTRL_0, FTP_CTRL_0_CUST_RST_INACTIVE | FTP_CTRL_0_CUST_REQ);
    waitnvmready();
    _i2c.write(SLAVE_ADDR, FTP_CTRL_1, FTP_CTRL_1_CUST_OPCODE_SOFT_PROGRAM_ARRAY);
    _i2c.write(SLAVE_ADDR, FTP_CTRL_0, FTP_CTRL_0_CUST_RST_INACTIVE | FTP_CTRL_0_CUST_REQ);
    waitnvmready();
    _i2c.write(SLAVE_ADDR, FTP_CTRL_1, FTP_CTRL_1_CUST_OPCODE_ERASE);
    _i2c.write(SLAVE_ADDR, FTP_CTRL_0, FTP_CTRL_0_CUST_RST_INACTIVE | FTP_CTRL_0_CUST_REQ);
    waitnvmready();

    // copy new data into temporary store
    _i2c.write(SLAVE_ADDR, 0x53, buf8, 8);
    _i2c.write(SLAVE_ADDR, FTP_CTRL_1, FTP_CTRL_1_CUST_OPCODE_SHIFT_IN_DATA_ON_PROGRAM_LOAD);
    _i2c.write(SLAVE_ADDR, FTP_CTRL_0, FTP_CTRL_0_CUST_RST_INACTIVE | FTP_CTRL_0_CUST_REQ);
    waitnvmready();

    // write new sector data to NVM
    _i2c.write(SLAVE_ADDR, FTP_CTRL_1, FTP_CTRL_1_CUST_OPCODE_PROGRAM);
    _i2c.write(SLAVE_ADDR, FTP_CTRL_0, FTP_CTRL_0_CUST_SECTOR(sector) | FTP_CTRL_0_CUST_RST_INACTIVE | FTP_CTRL_0_CUST_REQ);
    waitnvmready();
}

void STUSB4500::waitnvmready()
{
    uint8_t buf[1];
    do
    {
        _i2c.read(SLAVE_ADDR, FTP_CTRL_0, buf, 1);
    } while (buf[0] & FTP_CTRL_0_CUST_REQ);
}

uint8_t STUSB4500::currentF2I(float current)
{
    if (current < 0.5f)
        return 0;
    if (current < 3.0f)
        return (uint8_t)(4 * current) - 1;
    return std::min(15, (uint8_t)(2 * current) + 5);
}

float STUSB4500::currentI2F(uint8_t current)
{
    if (current == 0)
        return 0.0f;
    if (current < 11)
        return 0.25f * (current + 1);
    return 0.5f * (current - 5);
}

uint16_t STUSB4500::voltageF2I(float voltage)
{
    return (uint16_t)(voltage / 0.05f);
}

float STUSB4500::voltageI2F(uint16_t voltage)
{
    return voltage * 0.05f;
}

uint8_t STUSB4500::voltageMarginP2I(int percentage)
{
    return std::max(0, std::min(15, percentage));
}

int  STUSB4500::voltageMarginI2P(uint8_t storevalue)
{
    return storevalue;
}

