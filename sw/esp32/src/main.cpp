
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

#include "bitmap.h"
#include "color.h"
#include "fpgaconfigurator.h"
#include "graphics.h"
#include "ledpanel.h"
#include "ds3231.h"
#include "mpu6050.h"

#define LED_TEST      GPIO_NUM_33 // 1=on
#define BUTTON_TEST   GPIO_NUM_32 // 0=pressed

#define FPGA_SPI_CLK         GPIO_NUM_21
#define FPGA_SPI_MOSI        GPIO_NUM_25
#define FPGA_SPI_MISO        GPIO_NUM_22
#define FPGA_RESET           GPIO_NUM_19
#define I2C_SDA              GPIO_NUM_18
#define I2C_CLK              GPIO_NUM_5

SpiWrapper espSPI(SPI2_HOST, FPGA_SPI_CLK,FPGA_SPI_MOSI, FPGA_SPI_MISO, 5000000);
I2CWrapper espI2C(0, I2C_CLK, I2C_SDA);
FpgaConfigurator FpgaConfig(&espSPI, "/spiffs/toplevel_bitmap.bin", FPGA_RESET);


bool getButton()
{
  return !gpio_get_level(BUTTON_TEST);
}

void setLed(bool on)
{
  gpio_set_level(LED_TEST, on);
}

void animate()
{
    Bitmap drawing(10,10,3);
    drawing.fill(Color::black);    
    drawing.set(1,1,Color::red);
    drawing.set(2,1,Color::green);
    drawing.set(3,1,Color::blue);
    drawing.set(1,2,Color::green);
    drawing.set(2,2,Color::blue);
    drawing.set(3,2,Color::red);
    drawing.set(1,3,Color::blue);
    drawing.set(2,3,Color::red);
    drawing.set(3,3,Color::green);
    LedPanel panel(128, 64, espSPI);

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

        while (!getButton())
            vTaskDelay(10 / portTICK_PERIOD_MS);
        vTaskDelay(25 / portTICK_PERIOD_MS);
    }
}

void drawtext()
{
    LedPanel panel(128, 64, espSPI);
    Bitmap screen(128,64,3);
    screen.fill(Color::black);
    screen.copyTo(panel, 0, 0);

    Graphics g;
    g.setfont("luteouse", 30, 28);

    auto x = 5.0f;
    auto y = 32.0f;
    for (;;)
    {
        screen.fill(Color::black);
        g.text(screen, x, y, "02:41", Color::white);
        screen.copyTo(panel, 0, 0);

        x = x + 0.1;
        y = y + 0.05;
        while (!getButton())
            ;
    }
}

void read_tod()
{
    I2CWrapper i2c(0, I2C_SDA, I2C_CLK);        // note lines swapped on PCB
    i2c.start();

    DS3231 rtc(&i2c);

    for (;;)
    {
        rtc.readTimeFromChip();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    i2c.end();
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


void statemachine(void * parameter)
{
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
              case 1:
                read_orientation();
                break;
              case 2:
                read_tod();
                break;
              case 3:
                drawtext();
                break;
              case 4:
                animate();
                break;
              default:  // unknown command, restart
                break;
            }
            state = 0;
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
    gpio_set_direction(LED_TEST, GPIO_MODE_OUTPUT);
    gpio_set_direction(FPGA_SPI_CLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(FPGA_SPI_MOSI, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_TEST, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON_TEST, GPIO_MODE_INPUT);

    init_spiffs();

    FpgaConfig.configure();

    espSPI.start();

    xTaskCreate(statemachine, "test", 80000, NULL, 1, NULL);
}

extern "C" {

void app_main() 
{
  printf("Application starting\n");
  setup();      
}

}