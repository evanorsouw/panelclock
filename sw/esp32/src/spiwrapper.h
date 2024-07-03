
#ifndef SPIWRAPPER_H_
#define SPIWRAPPER_H_

#include "driver/gpio.h"
#include "esp_spiffs.h"
#include "driver/spi_master.h"

class SpiWrapper
{
private:
    spi_host_device_t _host;
    gpio_num_t _clk;
    gpio_num_t _mosi;
    gpio_num_t _miso;
    spi_device_handle_t _hspi;
    int _speed;
    int _mode;

public:
    SpiWrapper(spi_host_device_t host, gpio_num_t clk, gpio_num_t mosi, gpio_num_t miso, int speed)
    {
        _host = host;
        _clk = clk;
        _mosi = mosi;
        _miso = miso;
        _speed = speed;
        _mode = 0;  // spi mode 0 (rising edge
    }

    void start()
    {
        spi_bus_config_t bus_config = {0};
        bus_config.mosi_io_num = _mosi;
        bus_config.miso_io_num = _miso;
        bus_config.sclk_io_num = _clk;
        bus_config.quadwp_io_num = -1;
        bus_config.quadhd_io_num = -1;
        bus_config.max_transfer_sz = 3;

        ESP_ERROR_CHECK(spi_bus_initialize(_host, &bus_config, SPI_DMA_CH_AUTO));

        spi_device_interface_config_t devcfg = {0};
        devcfg.mode = _mode;
        devcfg.clock_speed_hz = _speed;
        devcfg.spics_io_num = -1;     
        devcfg.flags = SPI_DEVICE_HALFDUPLEX;
        devcfg.queue_size = 1;
        devcfg.pre_cb = NULL;
        devcfg.post_cb = NULL;

        ESP_ERROR_CHECK(spi_bus_add_device(_host, &devcfg, &_hspi));

        printf("intialized SPI bus=%d half duplex, mode=%d, mosi=%d, miso=%d, clk=%d, speed=%d\n", 
            _host, _mode, _mosi, _miso, _clk, _speed);
    }

    void end()
    {
        ESP_ERROR_CHECK(spi_bus_remove_device(_hspi));
        ESP_ERROR_CHECK(spi_bus_free(_host));
        printf("terminated SPI bus=%d\n", _host);
    }

    void write(uint8_t *buf, int n)
    {
        spi_transaction_t t = {
            .length = (size_t)(n * 8),
            .tx_buffer = buf
        };

        ESP_ERROR_CHECK(spi_device_polling_transmit(_hspi, &t));
    }
};

#endif