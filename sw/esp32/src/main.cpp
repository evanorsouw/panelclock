
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
#include "timesyncer.h"
#include "userinput_keys.h"
#include "wificlient.h"

#define BUTTON_SET    GPIO_NUM_35
#define BUTTON_UP     GPIO_NUM_32
#define BUTTON_DOWN   GPIO_NUM_34
#define BUTTON_BOOT   GPIO_NUM_0

#define FPGA_SPI_CLK         GPIO_NUM_21
#define FPGA_SPI_MOSI        GPIO_NUM_25
#define FPGA_SPI_MISO        GPIO_NUM_22
#define FPGA_RESET           GPIO_NUM_19
#define I2C_SDA              GPIO_NUM_18
#define I2C_CLK              GPIO_NUM_5

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

extern "C" {

void app_main() 
{
    printf("application starting\n");

    gpio_set_direction(FPGA_SPI_CLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(FPGA_SPI_MOSI, GPIO_MODE_OUTPUT);
    gpio_set_direction(FPGA_SPI_MOSI, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_SET, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_UP, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_DOWN, GPIO_MODE_INPUT);

    initNVS();
    init_spiffs();

    auto spi = new SpiWrapper(SPI2_HOST, FPGA_SPI_CLK, FPGA_SPI_MOSI, FPGA_SPI_MISO, 5400000, false);
    FpgaConfigurator FpgaConfig(spi, "/spiffs/toplevel_bitmap.bin", FPGA_RESET);
    FpgaConfig.configure();

    auto settings = new AppSettings();    
    auto panel = new LedPanel(settings->OnePanel() ? 64 : 128, 64, *spi, settings->get(settings->KeyFlipDisplay));

    auto graphics = new Graphics(panel->dx(), panel->dy());
    auto i2c = new I2CWrapper(I2C_NUM_0, I2C_CLK, I2C_SDA);
    i2c->start();
    auto rtc = new DS3231(i2c);
    auto system = new System(settings, rtc);
    auto userinput = new UserInputKeys(BUTTON_SET, BUTTON_UP, BUTTON_DOWN, BUTTON_BOOT, *system);
    auto environment = new EnvironmentWeerlive(system, settings->get(settings->KeyWeerliveKey), settings->get(settings->KeyWeerliveLocation));

    auto appdata = new ApplicationContext(*settings);

    auto appui = new Application(*appdata, *environment, *system, *userinput);
    auto bootui = new BootAnimations(*appdata, *environment, *system, *userinput);
    auto configui = new ConfigurationUI(*appdata, *environment, *system, *userinput);
    auto apprunner = new ApplicationRunner(*appdata, *panel, *bootui, *appui, *configui, *system, *graphics);
    auto timeupdater = new TimeSyncer(*rtc);

    xTaskCreate([](void*arg) { for(;;) ((ApplicationRunner*)arg)->renderTask();  }, "render", 80000, apprunner, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((ApplicationRunner*)arg)->displayTask(); }, "display", 4000, apprunner, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((UserInputKeys*)arg)->updateTask(); }, "userinput", 4000, userinput, 1, nullptr);

    printf("application initialized\n");
    printf("total free DRAM: %d (largest block: %d)\n", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    printf("total free IRAM: %d (largest block: %d)\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));
    printf("total free DMA: %d (largest block: %d)\n", heap_caps_get_free_size(MALLOC_CAP_DMA), heap_caps_get_largest_free_block(MALLOC_CAP_DMA));

    // use main thread for running periodic tasks.
    uint64_t timesync = appdata->starttimer();
    uint64_t weersync = appdata->starttimer();
    int timetimeout = 1 * 1000;
    int weathertimeout = 5 * 1000;
 
    for (;;)
    {
        // timer update from RTC
        if (appdata->timeoutAndRestart(timesync, timetimeout))
        {
            timetimeout = timeupdater->update();
        }
        // update weather data
        if (appdata->timeoutAndRestart(weersync, weathertimeout))
        {
            weathertimeout = environment->update();
        }
    }
}

}