
#include <stdio.h>
#include <math.h>
#include <cstdlib>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gpio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "events.h"
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
#include "environment_openweather.h"
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

    auto events = new Events();

    auto spi = new SpiWrapper(SPI2_HOST, FPGA_SPI_CLK, FPGA_SPI_MOSI, FPGA_SPI_MISO, 5400000, false);
    FpgaConfigurator FpgaConfig(spi, "/spiffs/ice40hx4k.bin", FPGA_RESET);
    FpgaConfig.configure();

    auto settings = new AppSettings();    
    auto panel = new LedPanel(settings->OnePanel() ? 64 : 128, 64, *spi, settings->get(settings->KeyFlipDisplay));

    auto graphics = new Graphics(panel->dx(), panel->dy());
    auto i2c = new I2CWrapper(I2C_NUM_0, I2C_CLK, I2C_SDA);
    i2c->start();
    auto rtc = new DS3231(i2c);
    auto system = new System(settings, rtc, events);
    auto userinput = new UserInputKeys(BUTTON_SET, BUTTON_UP, BUTTON_DOWN, BUTTON_BOOT, *system);
    auto environmentWL = new EnvironmentWeerlive(system, *settings, events->allocate("weerlive"));
    auto environmentOW = new EnvironmentOpenWeather(system, *settings, events->allocate("openweather"));
    auto environment = new EnvironmentSelector(std::vector<EnvironmentBase*>{ environmentWL, environmentOW }, *settings);

    auto appdata = new ApplicationContext(*settings);

    auto appui = new Application(*appdata, *environment, *system, *userinput);
    auto bootui = new BootAnimations(*appdata, *environment, *system, *userinput);
    auto configui = new ConfigurationUI(*appdata, *environment, *system, *userinput);
    auto otaui = new OTAUI(*appdata, *environment, *system, *userinput);
    auto apprunner = new ApplicationRunner(*appdata, *panel, *bootui, *appui, *configui, *otaui, *system, *graphics);
    auto timeupdater = new TimeSyncer(*rtc, *settings, events->allocate("timesync"));

    xTaskCreate([](void*arg) { for(;;) ((ApplicationRunner*)arg)->renderTask();  }, "render", 80000, apprunner, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((ApplicationRunner*)arg)->displayTask(); }, "display", 4000, apprunner, 1, nullptr);
    xTaskCreate([](void*arg) { for(;;) ((UserInputKeys*)arg)->updateTask(); }, "userinput", 4000, userinput, 1, nullptr);

    printf("application initialized\n");
    Diagnostic::printmeminfo();

    timeupdater->update(true);     // force immediate update from RTC

    // use main thread for running periodic tasks.
    for (;;)
    {
        events->wait();

        // established wifi forces weather update.
        if (system->wifiConnectedEvent()->wasSet() && system->wifiConnected())
        {
            environment->triggerUpdate();
            timeupdater->triggerUpdate();
        }

        if (environment->triggerUpdateEvent()->wasSet())
        {
            environment->update();
        }
        if (timeupdater->triggerUpdateEvent()->wasSet())
        {
            timeupdater->update(false); // ignore RTC when NTP is enabled
        }
    }
}

}