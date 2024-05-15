
#ifndef _FPGABITSTREAMLOADER_H_
#define _FPGABITSTREAMLOADER_H_

#include <stdio.h>

#include "driver/uart.h"
#include "esp_timer.h"
#include "esp_spiffs.h"

/// @brief class loads an (ICE40) bitstream into the ICE40 using SPI
class FpgaConfigurator
{
private:
    const char *_bitstreamfile;

public:
    FpgaConfigurator(const char *bitstream) : _bitstreamfile(bitstream) {}

    void configure()
    {
        FILE *fp = fopen(_bitstreamfile, "rb");

        int total = 0;
        int n = 0;
        char buf[1024];
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        {
            total += n;
        }

        printf("total is %d bytes in configuration\n", total);
        fflush(stdout);

        fclose(fp);
    }


private:
};

#endif
