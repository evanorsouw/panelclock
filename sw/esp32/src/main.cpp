
#include <stdio.h>
#include <math.h>
#include <cstdlib>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "i2cwrapper.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "spiwrapper.h"

#include "application.h"
#include "applicationrunner.h"
#include "appsettings.h"
#include "bitmap.h"
#include "bootanimations.h"
#include "color.h"
#include "ds3231.h"
#include "environment_weerlive.h"
#include "fpgaconfigurator.h"
#include "graphics.h"
#include "ledpanel.h"
#include "mpu6050.h"
#include "timesyncer.h"
#include "userinput_keys.h"
#include "wificlient.h"

#define LED_TEST      GPIO_NUM_33 // 1=on
#define BUTTON_SET    GPIO_NUM_32
#define BUTTON_UP     GPIO_NUM_35
#define BUTTON_DOWN   GPIO_NUM_34

#define FPGA_SPI_CLK         GPIO_NUM_21
#define FPGA_SPI_MOSI        GPIO_NUM_25
#define FPGA_SPI_MISO        GPIO_NUM_22
#define FPGA_RESET           GPIO_NUM_19
#define I2C_SDA              GPIO_NUM_18
#define I2C_CLK              GPIO_NUM_5

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

void initNVS()
{
    auto result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);
}

void configureFPGA(SpiWrapper *spi)
{
    FpgaConfigurator FpgaConfig(spi, "/spiffs/toplevel_bitmap.bin", FPGA_RESET);
    FpgaConfig.configure();
}

extern "C" {

void app_main() 
{
    printf("application starting\n");

    gpio_set_direction(FPGA_SPI_CLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(FPGA_SPI_MOSI, GPIO_MODE_OUTPUT);
    gpio_set_direction(FPGA_SPI_MOSI, GPIO_MODE_INPUT);
    gpio_set_direction(LED_TEST, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON_SET, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_UP, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_DOWN, GPIO_MODE_INPUT);

    initNVS();

    auto spi = new SpiWrapper(SPI2_HOST, FPGA_SPI_CLK, FPGA_SPI_MOSI, FPGA_SPI_MISO, 6000000, false);

    init_spiffs();

    configureFPGA(spi);
    auto settings = new AppSettings();
    auto panel = new LedPanel(128, 64, *spi);
    auto graphics = new Graphics(panel->dx(), panel->dy());
    //auto i2c = new I2CWrapper(0, I2C_SDA, I2C_CLK); // v2 pcb
    auto i2c = new I2CWrapper(0, I2C_CLK, I2C_SDA); // v3 pcb
    i2c->start();
    auto rtc = new DS3231(i2c);
    auto system = new System(settings, rtc);
    auto userinput = new UserInputKeys(BUTTON_SET, BUTTON_DOWN, BUTTON_UP, *system);
    auto environment = new EnvironmentWeerlive(system, settings->get(settings->KeyWeerliveKey), settings->get(settings->KeyWeerliveLocation));
    
    auto appdata = new ApplicationContext();

    auto appui = new Application(*appdata, *graphics, *environment, *system, *userinput);
    auto bootui = new BootAnimations(*appdata, *graphics, *environment, *system, *userinput);
    auto configui = new ConfigurationUI(*appdata, *graphics, *environment, *system, *userinput);
    auto apprunner = new ApplicationRunner(*appdata, *panel, *bootui, *appui, *configui, *system);
    auto timeupdater = new TimeSyncer(*rtc);

    xTaskCreate([](void*arg) { for(;;) ((ApplicationRunner*)arg)->renderTask();  }, "render", 80000, apprunner, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((ApplicationRunner*)arg)->displayTask(); }, "display", 2000, apprunner, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((TimeSyncer*)arg)->updateTask(); }, "timemgt", 2000, timeupdater, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((EnvironmentWeerlive*)arg)->updateTask(); }, "environment", 8000, environment, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((UserInputKeys*)arg)->updateTask(); }, "userinput", 2000, userinput, 1, nullptr);

    printf("application started\n");
}

}