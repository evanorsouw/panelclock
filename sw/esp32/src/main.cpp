
#include <stdio.h>
#include <math.h>
#include <cstdlib>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "spiwrapper.h"
#include "i2cwrapper.h"

#include "application.h"
#include "appsettings.h"
#include "bitmap.h"
#include "color.h"
#include "fpgaconfigurator.h"
#include "graphics.h"
#include "ledpanel.h"
#include "ds3231.h"
#include "environment_weerlive.h"
#include "mpu6050.h"
#include "max3421e.h"
#include "timeupdater.h"
#include "wificlient.h"

#define LED_TEST      GPIO_NUM_33 // 1=on
#define BUTTON_TEST   GPIO_NUM_32 // 0=pressed

#define USB_SPI_CLK          GPIO_NUM_14
#define USB_SPI_MOSI         GPIO_NUM_12
#define USB_SPI_MISO         GPIO_NUM_13
#define USB_SPI_CS           GPIO_NUM_27
#define USB_SPI_INT          GPIO_NUM_15

#define FPGA_SPI_CLK         GPIO_NUM_21
#define FPGA_SPI_MOSI        GPIO_NUM_25
#define FPGA_SPI_MISO        GPIO_NUM_22
#define FPGA_RESET           GPIO_NUM_19
#define I2C_SDA              GPIO_NUM_18
#define I2C_CLK              GPIO_NUM_5

bool getButton()
{
  return !gpio_get_level(BUTTON_TEST);
}

void waitKey()
{
    while (!getButton());
    vTaskDelay(100 / portTICK_PERIOD_MS);
    while (getButton());
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void setLed(bool on)
{
  gpio_set_level(LED_TEST, on);
}

void dousb()
{
    printf("*** MAX3421E ***\n");

    SpiWrapper spi(SPI3_HOST, USB_SPI_CLK, USB_SPI_MOSI, USB_SPI_MISO, 100000, true);
    spi.start();

    MAX3421E usb(&spi, USB_SPI_CS);

    printf("constructed\n");

    usb.start();
    printf("started\n");

    uint8_t last_hrsl = 0xFE;
    uint8_t last_hirq = 0xFE;
    uint8_t last_mode = 0xFE;
    uint8_t last_revision = 0xFE;
    for (;;)
    {
        usb.writeReg(MODE, MODE_HOST);
        auto hrsl = usb.readReg(HRSL);  // F8 00
        auto hirq = usb.readReg(HIRQ);  // C8 00
        auto mode = usb.readReg(MODE);  // D8 00
        auto revision = usb.readReg(REVISION);

        if (hrsl != last_hrsl || hirq != last_hirq || last_mode != mode || revision != last_revision)
        {
            last_hrsl = hrsl;
            last_hirq = hirq;
            last_mode = mode;
            last_revision = revision;
            printf("hrsl=0x%02X, hirq=%s,%s,%s,%s,%s,%s,%s,%s, mode=0x%02X, revion=0x%02X\n", 
            hrsl, 
            (hirq&HIRQ_BUSEVENT) ? "BUSEVENT" : "-",
            (hirq&HIRQ_RWU) ? "RWUIRQ" : "-",   
            (hirq&HIRQ_RCVDAV) ? "RCVDAV" : "-",
            (hirq&HIRQ_SNDBAV) ? "SNDBAV" : "-",
            (hirq&HIRQ_SUSDN) ? "SUSDN" : "-",
            (hirq&HIRQ_CONDET) ? "CONDET" : "-",
            (hirq&HIRQ_FRAME) ? "FRAME" : "-", 
            (hirq&HIRQ_HXFRDN) ? "HXFRDN" : "-", 
            mode, revision);

            usb.writeHIRQ(hirq);
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void read_orientation()
{
    I2CWrapper i2c(0, I2C_CLK, I2C_SDA);
    i2c.start();

    MPU6050 accel(&i2c);
    accel.start();

    for (;;)
    {
        accel.readAngles();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    i2c.end();
}


void init_spiffs()
{
    esp_vfs_spiffs_conf_t spiffs_config = {
        .base_path = "/spiffs",
        .partition_label = nullptr,
        .max_files = 5,
        .format_if_mount_failed = false
    };
    esp_vfs_spiffs_register(&spiffs_config);
}

void configureFPGA(SpiWrapper *spi)
{
    FpgaConfigurator FpgaConfig(spi, "/spiffs/toplevel_bitmap.bin", FPGA_RESET);
    FpgaConfig.configure();
}

void foregroundtasks(void * parameter)
{
    for(;;) ((Application*)parameter)->renderTask();
}

void backgroundtasks(void *parameter)
{
    for (;;) ((Application*)parameter)->displayTask();
}

extern "C" {

void app_main() 
{
    printf("Application starting\n");

    gpio_set_direction(LED_TEST, GPIO_MODE_OUTPUT);
    gpio_set_direction(FPGA_SPI_CLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(FPGA_SPI_MOSI, GPIO_MODE_OUTPUT);
    gpio_set_direction(FPGA_SPI_MOSI, GPIO_MODE_INPUT);
    gpio_set_direction(LED_TEST, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON_TEST, GPIO_MODE_INPUT);

    auto spi = new SpiWrapper(SPI2_HOST, FPGA_SPI_CLK, FPGA_SPI_MOSI, FPGA_SPI_MISO, 6000000, false);

    init_spiffs();
    configureFPGA(spi);
    
    auto settings = new AppSettings();
    settings->loadSettings();

    auto graphics = new Graphics();
    auto panel = new LedPanel(128, 64, *spi);
    auto i2c = new I2CWrapper(0, I2C_SDA, I2C_CLK);        // note lines swapped on PCB
    i2c->start();
    auto rtc = new DS3231(i2c);
    auto system = new System(settings);

    auto environment = new EnvironmentWeerlive(system, settings->WeerliveKey(), settings->WeerliveLocation());
    
    auto app = new Application(*graphics, *panel, *rtc, *environment, *system);
    auto timeupdater = new TimeUpdater(*rtc);

    xTaskCreate([](void*arg) { for(;;) ((Application*)arg)->renderTask();  }, "render", 80000, app, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((Application*)arg)->displayTask(); }, "display", 4000, app, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((TimeUpdater*)arg)->updateTask(); }, "timemgt", 4000, timeupdater, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((EnvironmentWeerlive*)arg)->updateTask(); }, "environment", 12000, environment, 1, nullptr);
}

}