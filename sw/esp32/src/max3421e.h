
#ifndef _MAX3421E_H_
#define _MAX3421E_H_

#include <freertos/task.h>
#include "spiwrapper.h"

#define RCVFIFO       1
#define SNDFIFO       2
#define SUBFIFO       4
#define RCVBC         6
#define SNDBC         7
#define USBIRQ        13
#define USBIEN        14
#define USBCTL        15
#define CPUCTL        16
#define PINCTL        17
#define REVISION      18
#define HIRQ          25
#define HIEN          26
#define MODE          27
#define PERADDR       28
#define HCTL          29
#define HXFR          30
#define HRSL          31

#define READ_REG(r)         ((uint8_t)(((r) & 0x1F) << 3))
#define WRITE_REG(r)        ((uint8_t)(READ_REG(r) | 0x02))
  
#define USBCTL_CHIPRES      0x20

#define CPUCTL_PULSEWID(w)  (((w)&3)<<6)
#define CPUCTL_IE           0x01
  
#define HXFR_SETUP          0x01
#define HXFR_BULKIN(ep)     (0x00 | ((ep)&0xF))
#define HXFR_BULKOUT(ep)    (0x20 | ((ep)&0xF))
#define HXFR_HS_IN          0x80
#define HXFR_HS_OUT         0xA0
#define HXFR_ISO_IN(ep)     (0x40 | ((ep) & 0xF))
#define HXFR_ISO_OUT(ep)    (0x60 | ((ep) & 0xF))

#define HIRQ_BUSEVENT       (1<<0)      // Bus Event
#define HIRQ_RWU            (1<<1)      // Remote WakeUp
#define HIRQ_RCVDAV         (1<<2)      // Receive Data Available
#define HIRQ_SNDBAV         (1<<3)      // Send Buffer Available
#define HIRQ_SUSDN          (1<<4)      // Suspend
#define HIRQ_CONDET         (1<<5)      // Connect/Disconnect
#define HIRQ_FRAME          (1<<6)      // Frame at 1ms interval
#define HIRQ_HXFRDN         (1<<7)      // Host Transfer Done

#define HCTL_BUSRST         (1<<0)
#define HCTL_FRMRST         (1<<1)
#define HCTL_SAMPLEBUS      (1<<2)
#define HCTL_SIGRSM         (1<<3)

#define HRSL_JSTATUS        0x80
#define HRSL_KSTATUS        0x40
#define HRSL_SNDTOGRD(x)    (((x)&3)<<4)
#define HRSL_HRSLT(x)       ((x) & 0xF)

#define USBIRQ_OSCOKIRQ     (1<<0)
#define USBIRQ_NOVBUSIRQ    (1<<5)
#define USBIRQ_VBUSIRQ      (1<<6)

#define MODE_DPPULLN        0x80
#define MODE_DMPULLN        0x40
#define MODE_DELAYISOD      0x20
#define MODE_SEPIRQ         0x10
#define MODE_SOFKAENAB      0x08
#define MODE_HUBPRE         0x04
#define MODE_SPEED          0x02
#define MODE_HOST           0x01

#define PINCTL_FDUPSPI      0x10
#define PINCTL_INTLEVEL     0x08

#define HIEN_URESDNIE       0x01
   
#define HRSLT_SUCCESS       0
#define HRSLT_BUSY          1
#define HRSLT_NAK           4
#define HRSLT_STALL         5

struct endpoint {
    uint8_t addr;
    uint8_t ep;
    uint8_t dir;
    uint8_t maxpktsize;
};

class MAX3421E
{
private:
    SpiWrapper *_spi;
    gpio_num_t _cs;
    uint8_t _hirqReg;

public:
    MAX3421E(SpiWrapper *spi, gpio_num_t cs)
    {
        _spi = spi;
        _cs = cs;
    }

    void start()
    {
        gpio_set_direction(_cs, GPIO_MODE_OUTPUT);

        writeReg(PINCTL, PINCTL_FDUPSPI | PINCTL_INTLEVEL);
        auto revision = readReg(REVISION);
        printf("MAX3421E revision %d\n", revision);

        // reset device
        printf("reset MAX3421E\n");
        writeReg(USBCTL, USBCTL_CHIPRES);
        writeReg(USBCTL, 0x00);
        waitForIrq(USBIRQ, USBIRQ_OSCOKIRQ, 0xFFFFF);

        // setup host
        writeReg(MODE, MODE_HOST | MODE_DPPULLN | MODE_DMPULLN);
        writeReg(CPUCTL, CPUCTL_IE);

        writeReg(HCTL, HCTL_BUSRST | HCTL_FRMRST);

        // clear all pending IRQ's
        writeReg(HIRQ, 0xFF);
    }

    void end()
    {
        _spi->end();
    }

    /**
     * @return the last status byte received from the MAX3421E
     */
    uint8_t hirq() const { return _hirqReg; }

    /**
     * \brief write a value to a register.
     * 
     * \param reg the register to write
     * \param value the value
     */
    void writeReg(uint8_t reg, uint8_t value)
    {
        uint8_t wbuf[2] = { READ_REG(reg), value };

        select();
        _spi->transfer(wbuf, 2, &_hirqReg, 1);
        deselect();
    }

    /**
     * @brief read a value from a register.
     * the status register that is also returned as a result of this request
     * is available via the status() method.
     * 
     * @param reg the register to read
     * @return uint8_t the value of the register.
     */
    uint8_t readReg(uint8_t reg)
    {
        uint8_t wbuf[2] = { READ_REG(reg), 0 };
        uint8_t rbuf[2] = { 0xAA, 0x55 };
        
        select();
        _spi->transfer(wbuf, 2, rbuf, 2);
        deselect();

        _hirqReg = rbuf[0];
        return rbuf[1];
    }

    void writeFifo(uint8_t reg, uint8_t *wbuf, int nw)
    {
        uint8_t cmd = WRITE_REG(reg);

        select();
        _spi->transfer(&cmd, 1, &_hirqReg, 1);
        _spi->transfer(wbuf, nw, nullptr, 0);
        deselect();
    }

    void readFifo(uint8_t reg, uint8_t *rbuf, int nr)
    {
        uint8_t cmd = READ_REG(reg);

        select();
        _spi->transfer(&cmd, 1, &_hirqReg, 1);
        _spi->transfer(rbuf, nr, nullptr, 0);
        deselect();
    }

    void writeHIEN(uint8_t value) { writeReg(HIEN, value); }
    void writeHIRQ(uint8_t value) { writeReg(HIRQ, value); }
    void writeMODE(uint8_t value) { writeReg(MODE, value); }
    void writePERADDR(uint8_t value) { writeReg(PERADDR, value); }
    void writeHXFR(uint8_t value) { writeReg(HXFR, value); }
    void writeSNDBC(uint8_t value) { writeReg(SNDBC, value); }

    bool waitForIrq(uint8_t reg, uint8_t mask, int maxticks = 100)
    {
        printf("waiting for irq [%d] & 0x%02X ... ", reg, mask);
        fflush(stdout);

        int ticks = 0;
        while ((readReg(reg) & mask) == 0)
        {
            if (ticks++ > maxticks)
            {
                printf("timeout\n");
                return false;
            }
            vTaskDelay(1);
        }
        printf("got irq after %d ticks\n", ticks);

        // clear the IRQ
        writeReg(reg, mask);
        return true;
    }

private:
    void select() { gpio_set_level(_cs, 0); }
    void deselect() { gpio_set_level(_cs, 1); }
};

#endif
