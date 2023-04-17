// #define NDEBUG

#include "bitbang/timing.h"
#include "graphics.h"
#include "hardware/adc.h"

#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_system.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
#include <rom/ets_sys.h>
#include <sdkconfig.h>

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

// GPIO tested & working on pins 27, 33, 32

// #define SPEAKER_PIN 27

// FreeRTOS entry point instead of `main`, but using none of FreeRTOS's other features.
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

  graphics_init();

  adc_init();

  timing_init();
  uint64_t next_timer = 0;
  do {
    //     printf("t=%llu\r\n", timing_get_clock());
    printf("%hu\r\n", adc_poll());
    if (timing_get_clock() < next_timer) { printf("Waiting...\r\n"); }
    do {
    } while (timing_get_clock() < next_timer);
    ++next_timer;
  } while (1);

  ets_delay_us(10000000); // 10s

  esp_restart();
}
