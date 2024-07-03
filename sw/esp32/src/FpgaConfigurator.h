
#ifndef _FPGABITSTREAMLOADER_H_
#define _FPGABITSTREAMLOADER_H_

#include <string.h>
#include <stdio.h>

#include "spiwrapper.h"

/// @brief class loads an (ICE40) bitstream into the ICE40 using SPI
class FpgaConfigurator
{
private:
    SpiWrapper *_spi;
    const char *_bitstreamfile;
    gpio_num_t _ioReset;

public:
    FpgaConfigurator(SpiWrapper *spi, const char *bitstream, gpio_num_t reset) 
        : _spi(spi), _bitstreamfile(bitstream), _ioReset(reset) {}

    void configure()
    {
        _spi->start();

        // perform reset
        gpio_set_direction(_ioReset, GPIO_MODE_OUTPUT);
        gpio_set_level(_ioReset, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(_ioReset, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);

        FILE *fp = fopen(_bitstreamfile, "rb");

        int total = 0;
        int n = 0;
        uint8_t buf[1024];
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        {
            total += n;
            _spi->write(buf, n);
        }

        memset(buf, 0, 128);
        _spi->write(buf, 128);

        printf("configured fpga with %d bytes\n", total);

        _spi->end();
        fclose(fp);
    }
};

#endif
