
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
#include "bitmap.h"
#include "ledpanel.h"

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

    Bitmap drawing(10,10);
    drawing.fill(0x000000);    
    drawing.set(1,1,0xFF0000);
    drawing.set(2,1,0x00FF00);
    drawing.set(3,1,0x0000FF);
    drawing.set(1,2,0x00FF00);
    drawing.set(2,2,0x0000FF);
    drawing.set(3,2,0xFF0000);
    drawing.set(1,3,0x0000FF);
    drawing.set(2,3,0xFF0000);
    drawing.set(3,3,0x00FF00);
    LedPanel panel(128, 64, espSpi);

    int dir = 1;
    int x = 1;
    int y = 1;
    for (;;)
    {
        drawing.copyTo(panel, x, y);

        x += dir;
        if (x == 100 || x == 1)
        {
            dir = -dir;
            y++;
            if (y == 50)
                y = 0;
        }

        if (false)
        {
            while (!getButton())
                vTaskDelay(10 / portTICK_PERIOD_MS);
            while (getButton())
                vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        else
        {
            vTaskDelay(25 / portTICK_PERIOD_MS);
        }
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