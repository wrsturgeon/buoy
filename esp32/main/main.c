#include "bitbang/lcd.h"

#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_system.h>
#include <rom/ets_sys.h>
#include <sdkconfig.h>

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

// GPIO tested & working on pins 27, 33, 32

// #define SPEAKER_PIN 27

void app_main(void) {

  { // Print runtime chip info
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    uint32_t flash_size;
    ESP_ERROR_CHECK(esp_flash_get_size(NULL, &flash_size));
    char chipname[sizeof CONFIG_IDF_TARGET];
    for (uint8_t i = 0; i < (sizeof CONFIG_IDF_TARGET) - 1; ++i) { chipname[i] = toupper(CONFIG_IDF_TARGET[i]); }
    chipname[(sizeof CONFIG_IDF_TARGET) - 1] = 0; // null terminator
    printf(
        "Running on a %d-core %s with WiFi%s%s%s, silicon revision v%d.%d, %" PRIu32 "MB %s flash, minimum free heap size of %" PRIu32
        "B\r\nNDEBUG "
#ifdef NDEBUG
        "defined(!)"
#else
        "undefined"
#endif
        " during compilation\r\n",
        chip_info.cores,
        chipname,
        (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
        (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
        (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "",
        chip_info.revision / 100U,
        chip_info.revision % 100U,
        flash_size >> 20U,
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external",
        esp_get_minimum_free_heap_size());
  }

  spi_init();
  lcd_init();

  ets_delay_us(100000); // 0.1s

#define x0 32
#define y0 32
#define x1 64
#define y1 64
  spi_open();
  SPI_COMMAND(ST7735_CASET, 4, 0, 0, x0, 0, x1);
  SPI_COMMAND(ST7735_RASET, 4, 0, 0, y0, 0, y1);
  SPI_COMMAND(ST7735_RAMWR, 0, 5);
  for (uint16_t ij = 0; ij != (uint16_t)(((uint16_t)(y1 - y0 + 1)) * (uint16_t)(x1 - x0 + 1)); ++ij) { spi_send_16b(0b1111100000000000); }
  spi_close();
#undef y1
#undef x1
#undef y0
#undef x0

  ets_delay_us(1000000); // 1s

  // GPIO_ENABLE_OUTPUT(SPEAKER_PIN);
  // do {
  //   printf("Pulling low...\r\n");
  //   GPIO_PULL_LO(SPEAKER_PIN);
  //   ets_delay_us(1000000);

  //   printf("Pulling high...\r\n");
  //   GPIO_PULL_HI(SPEAKER_PIN);
  //   ets_delay_us(1000000);
  // } while (1);

  ets_delay_us(1000000); // 1s

  esp_restart();
}
