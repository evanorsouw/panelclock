
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s_std.h>
#include <driver/gpio.h>
#include <driver/spi_common.h>
#include <driver/spi_master.h>

#define LED           GPIO_NUM_2
#define I2S_DOUT      GPIO_NUM_23
#define I2S_BCLK      GPIO_NUM_27
#define I2S_LRC       GPIO_NUM_26

i2s_chan_handle_t     tx_chan;
TaskHandle_t          I2SSoundTask;

void toggleLED(void * parameter)
{
  for(;;)
  {
    gpio_set_level(LED, 1);    
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(LED, 0);    
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

#define I2S_BUFSIZE 16384
static uint8_t i2sbuffer[I2S_BUFSIZE];
static int i2sireadbuf;
static int i2siwritebuf;

uint16_t phase = 0;
int16_t freq1 = 1000;
int16_t freq2 = 1003;

void i2s_fillbuffers()
{
  auto filled = i2siwritebuf - i2sireadbuf;
  if (filled < 0) filled += I2S_BUFSIZE;
  auto n = I2S_BUFSIZE - filled - 2;
  
  for (int i=0; i<n; i+=2)
  {
    int16_t sample1 = 400 * cos(3.1415f * freq1 * phase / 44100);
    int16_t sample2 = 400 * cos(3.1415f * freq2 * phase / 44100);
    phase = (phase + 1) % 44100;
    int16_t sample = sample1 + sample2;
    i2sbuffer[i2siwritebuf++] = sample % 256;
    i2sbuffer[i2siwritebuf++] = sample / 256;
    if (i2siwritebuf == I2S_BUFSIZE)
      i2siwritebuf = 0;
  }
}

void i2s_sound(void *param)
{
  printf("i2s_sound()\n");

  i2sireadbuf = 0;
  i2siwritebuf = 0;

  i2s_channel_enable(tx_chan);

  while (1) 
  {
    i2s_fillbuffers();
    auto pending = i2siwritebuf - i2sireadbuf;
    if (pending < 0) pending += I2S_BUFSIZE;
    auto pendingatend = I2S_BUFSIZE - i2sireadbuf;
    auto towrite =  (pendingatend < pending) ? pendingatend : pending;
    if (towrite > 4096) towrite = 4096;

    size_t written = 0;
    auto result = i2s_channel_write(tx_chan, i2sbuffer + i2sireadbuf, towrite, &written, portTICK_PERIOD_MS);
    printf("write: result=%d, read=%d, write=%d, towrite=%d, written=%d\n", result, i2sireadbuf, i2siwritebuf, towrite, written);

    i2sireadbuf = (i2sireadbuf + written) % I2S_BUFSIZE;
  }
}

void i2s_init()
{
  i2s_chan_config_t tx_chan_cfg = {
      .id = I2S_NUM_AUTO,
      .role = I2S_ROLE_MASTER,
      .dma_desc_num = 4,
      .dma_frame_num = 2000,
      .auto_clear = true
    };
    i2s_new_channel(&tx_chan_cfg, &tx_chan, nullptr);
   
    i2s_std_config_t tx_std_cfg = {
        .clk_cfg = {
          .sample_rate_hz = 44100,
          .clk_src = I2S_CLK_SRC_DEFAULT,
          .mclk_multiple = I2S_MCLK_MULTIPLE_256
        },
        .slot_cfg = { 
          .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT, 
          .slot_bit_width = I2S_SLOT_BIT_WIDTH_16BIT, 
          .slot_mode = I2S_SLOT_MODE_MONO, 
          .slot_mask = I2S_STD_SLOT_BOTH,
          .ws_width = 16, 
          .ws_pol = false, 
          .bit_shift = false, 
          .msb_right = false
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = I2S_BCLK,
            .ws   = I2S_LRC,
            .dout = I2S_DOUT,
            .din  = I2S_DOUT,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    gpio_set_direction(I2S_DOUT, GPIO_MODE_OUTPUT);
    gpio_set_direction(I2S_BCLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(I2S_LRC, GPIO_MODE_OUTPUT);

    i2s_channel_init_std_mode(tx_chan, &tx_std_cfg);

    vTaskDelay(3000 / portTICK_PERIOD_MS);
}

spi_device_handle_t max3421_spi;

void MAX3421_init()
{
  auto host = SPI2_HOST;  // aka VSPI, MOSI=23,MISO=19,CLK=18,SS=5
  static spi_bus_config_t config = {
    .mosi_io_num = 18,
    .miso_io_num = 19,
    .sclk_io_num = 21,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 32
  };
  static spi_device_interface_config_t dev_config = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,
    .cs_ena_posttrans = 3,
    .clock_speed_hz = 100000,
    .spics_io_num = 5,
    .flags = 0,
    .queue_size = 3,
    .pre_cb = nullptr,
    .post_cb = nullptr   
  };
  ESP_ERROR_CHECK(spi_bus_initialize(host, &config, SPI_DMA_CH_AUTO));
  ESP_ERROR_CHECK(spi_bus_add_device(host, &dev_config, &max3421_spi));
}

uint16_t MAX3421_writeReg(uint8_t reg, uint8_t value)
{
  spi_transaction_t t = { 0 };
  t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  t.tx_data[0] = (reg << 3) | 0x02;
  t.tx_data[1] = value;
  t.length = 2*8;
  ESP_ERROR_CHECK(spi_device_transmit(max3421_spi, &t));
  uint16_t rcvd = ((uint16_t)(t.rx_data[0]) << 8) | t.rx_data[1];
  printf("write %02X%02X, received=%04X\n", t.tx_data[0], t.tx_data[1], rcvd);
  return rcvd;
}

uint16_t MAX3421_readReg(uint8_t reg)
{
  spi_transaction_t t = { 0 };
  t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
  t.tx_data[0] = (reg << 3);
  t.tx_data[1] = 0;
  t.length = 2*8;
  ESP_ERROR_CHECK(spi_device_transmit(max3421_spi, &t));
  uint16_t rcvd = ((uint16_t)(t.rx_data[0]) << 8) | t.rx_data[1];
  printf("read %02X%02X, received=%04X\n", t.tx_data[0], t.tx_data[1], rcvd);
  return rcvd;
}


void interactWithMax3421(void * parameter)
{
  printf("---------------\n");
  printf("starting MAX3421\n");

  MAX3421_init();
  printf("init ok\n");

  for (;;)
  {
    MAX3421_writeReg(27, 0x01);
    MAX3421_writeReg(17, 0x10);
    MAX3421_readReg(27);
    MAX3421_readReg(13);
    MAX3421_readReg(14);
    MAX3421_readReg(15);
    MAX3421_readReg(13);

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void setup() 
{
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);

  //i2s_init();

  //xTaskCreate(i2s_sound, "I2SSound", 5000, NULL, 5, &I2SSoundTask);  
  
  xTaskCreate(toggleLED, "Toggle", 4000, NULL, 1, NULL);
  xTaskCreate(interactWithMax3421, "MAX3421", 4000, NULL, 1, NULL);
}

extern "C" {

void app_main() 
{
    setup();      
}

}