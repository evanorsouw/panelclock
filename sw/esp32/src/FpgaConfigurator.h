
#ifndef _FPGABITSTREAMLOADER_H_
#define _FPGABITSTREAMLOADER_H_

#include <string.h>
#include <stdio.h>

#include "driver/uart.h"
#include "esp_timer.h"
#include "esp_spiffs.h"
#include "driver/spi_master.h"

/// @brief class loads an (ICE40) bitstream into the ICE40 using SPI
class FpgaConfigurator
{
private:
    const char *_bitstreamfile;
    const gpio_num_t GPIO_MOSI = GPIO_NUM_25;
    const gpio_num_t GPIO_CLK  = GPIO_NUM_21;
    const gpio_num_t GPIO_RESET = GPIO_NUM_19;

public:
    FpgaConfigurator(const char *bitstream) 
        : _bitstreamfile(bitstream) {}

    void configure()
    {
        spi_bus_config_t bus_config = {
            .mosi_io_num = GPIO_MOSI,
            .miso_io_num = -1,
            .sclk_io_num = GPIO_CLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 32
        };

        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO));

        spi_device_handle_t hSpi;
        spi_device_interface_config_t devcfg = {
            .mode = 0,                  //SPI mode 0
            .clock_speed_hz = 1000000,  // 1 MHz
            .spics_io_num = -1,     
            .flags = SPI_DEVICE_HALFDUPLEX,
            .queue_size = 1,
            .pre_cb = NULL,
            .post_cb = NULL
        };

        ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &hSpi));

        // perform reset
        gpio_set_direction(GPIO_RESET, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_RESET, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_RESET, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);


        FILE *fp = fopen(_bitstreamfile, "rb");

        int total = 0;
        int n = 0;
        char buf[1024];
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        {
            total += n;
            writeSpi(hSpi, buf, n);
        }

        memset(buf, 0, 128);
        writeSpi(hSpi, buf, 128);

        ESP_ERROR_CHECK(spi_bus_remove_device(hSpi));
        ESP_ERROR_CHECK(spi_bus_free(SPI2_HOST));

        printf("total is %d bytes in configuration\n", total);
        fflush(stdout);

        fclose(fp);
    }

private:
    void writeSpi(spi_device_handle_t hSpi, char *buf, int n)
    {
        spi_transaction_t t = {
            .length = (size_t)(n * 8),
            .tx_buffer = buf
        };

        ESP_ERROR_CHECK(spi_device_polling_transmit(hSpi, &t));
    }
};

#endif
