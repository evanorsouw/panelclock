
#include <stdio.h>
#include <math.h>
#include <cstdlib>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "SpiWrapper.h"

#include "FpgaConfigurator.h"

#define LED_TEST      GPIO_NUM_33 // 1=on
#define BUTTON_TEST   GPIO_NUM_32 // 0=pressed

#define FPGA_SPI_CLK         GPIO_NUM_21
#define FPGA_SPI_MOSI        GPIO_NUM_25
#define FPGA_SPI_MISO        GPIO_NUM_22
#define FPGA_RESET           GPIO_NUM_19

SpiWrapper espSpi(SPI2_HOST, FPGA_SPI_CLK,FPGA_SPI_MOSI, FPGA_SPI_MISO);
FpgaConfigurator FpgaConfig(&espSpi, "/spiffs/toplevel_bitmap.bin", FPGA_RESET);


bool getButton()
{
  return !gpio_get_level(BUTTON_TEST);
}

void setLed(bool on)
{
  gpio_set_level(LED_TEST, on);
}

void SendIntermittendSpiPackets()
{
    espSpi.start();

    uint8_t buffer[16];
    bool led = true;

    int n = 0;
    int color = 0;
    while (!getButton())
    {
        auto i = 0;

        auto dx = 1;
        auto dy = 64;
        auto x = n;
        auto y = 0;

        buffer[i++] = 2;
        buffer[i++] = x;
        buffer[i++] = y;
        buffer[i++] = dx;
        buffer[i++] = dy;
        buffer[i++] = color;
        buffer[i++] = color;
        buffer[i++] = color;
        espSpi.write(buffer, 8);

        setLed(led);
        led = !led;

        vTaskDelay(10 / portTICK_PERIOD_MS);

        n++;
        if (n == 128)
            n = 0;
            
        color++;
        if (color == 256)
            color = 0;
    }
    espSpi.end();
}

void statemachine(void * parameter)
{
  gpio_set_direction(LED_TEST, GPIO_MODE_OUTPUT);
  gpio_set_direction(FPGA_SPI_CLK, GPIO_MODE_OUTPUT);
  gpio_set_direction(FPGA_SPI_MOSI, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_TEST, GPIO_MODE_OUTPUT);
  gpio_set_direction(BUTTON_TEST, GPIO_MODE_INPUT);

  auto interval = 50;
  auto state = 0;
  auto lastButton = 0;
  auto stable = 0;
  auto choice = 0;
  for(;;)
  {
    vTaskDelay(interval / portTICK_PERIOD_MS);
    auto button = getButton();
    if (button == lastButton)
    {
      stable += interval;
      switch (state)
      {
        case 0: // idle
          if (stable > 1000 && button)
          {
            state = 1;
            choice = 0;
            setLed(false);
          }
          break;
        case 1: // counting key presses
          if (choice == 0 && stable > 15000)
          {
            state = 0;
          }
          else if (choice > 0 && stable > 1500)
          {
            switch (choice)
            {
              case 2:
                SendIntermittendSpiPackets();
                break;
              default:  // unknown command, restart
                state = 0;
                break;
            }
          }
          break;        
      }
    }
    else
    {
      switch (state)
      {
        case 0:
          setLed(button);
          break;
        case 1:
          setLed(button);
          if (button && !lastButton)
          {
            choice++;
          }
      }
      lastButton = button;      
      stable = 0;
    }
  }
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

void setup() 
{
    init_spiffs();
    
    FpgaConfig.configure();

    xTaskCreate(statemachine, "test", 4000, NULL, 1, NULL);
}

extern "C" {

void app_main() 
{
  printf("Application starting\n");
  setup();      
}

}