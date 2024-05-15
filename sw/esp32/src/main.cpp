
#include <stdio.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"

#include "FpgaConfigurator.h"

#define LED_TEST      GPIO_NUM_33 // 1=on
#define BUTTON_TEST   GPIO_NUM_32 // 0=pressed

bool getButton()
{
  return !gpio_get_level(BUTTON_TEST);
}

void setLed(bool on)
{
  gpio_set_level(LED_TEST, on);
}

void downloadFpgaBinary()
{
  for (;;)
  {
    setLed(true);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    setLed(false);
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

void statemachine(void * parameter)
{
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
              case 2: // download FPGA binary
                downloadFpgaBinary();
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

FpgaConfigurator FpgaConfig("/spiffs/toplevel_bitmap.bin");

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