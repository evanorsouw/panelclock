
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
#include "timeupdater.h"
#include "wificlient.h"

#define LED_TEST      GPIO_NUM_33 // 1=on
#define BUTTON_TEST   GPIO_NUM_32 // 0=pressed

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
    printf("application starting\n");

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

    auto panel = new LedPanel(128, 64, *spi);
    auto graphics = new Graphics(panel->dx(), panel->dy());
    auto i2c = new I2CWrapper(0, I2C_SDA, I2C_CLK);        // note lines swapped on PCB
    i2c->start();
    auto rtc = new DS3231(i2c);
    auto system = new System(settings);

    auto environment = new EnvironmentWeerlive(system, settings->WeerliveKey(), settings->WeerliveLocation());
    
    auto app = new Application(*graphics, *panel, *environment, *system);
    auto timeupdater = new TimeUpdater(*rtc);

    xTaskCreate([](void*arg) { for(;;) ((Application*)arg)->renderTask();  }, "render", 80000, app, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((Application*)arg)->displayTask(); }, "display", 4000, app, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((TimeUpdater*)arg)->updateTask(); }, "timemgt", 4000, timeupdater, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((EnvironmentWeerlive*)arg)->updateTask(); }, "environment", 12000, environment, 1, nullptr);

    printf("application started\n");
}

}