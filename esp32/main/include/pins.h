#ifndef PINS_H
#define PINS_H

// Convenience macros (I wrote this file).
// Adafruit Feather V2 with an onboard (plain) ESP32:
// https://learn.adafruit.com/assets/112834

#define FEATHER_A0 26
#define FEATHER_A1 25
#define FEATHER_A2 34
#define FEATHER_A3 39
#define FEATHER_A4 36
#define FEATHER_A5 4

#define FEATHER_13 13
#define FEATHER_12 12
#define FEATHER_27 27
#define FEATHER_33 33
#define FEATHER_15 15
#define FEATHER_32 32
#define FEATHER_14 14

#define FEATHER_HSPI_BUS 1   // HSPI = SPI2_HOST = 1 (ig 0-indexed?); see ~/esp/esp-idf/components/hal/include/hal/spi_types.h:22 and https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html
#define FEATHER_HSPI_MISO 12 // aka Q
#define FEATHER_HSPI_MOSI 13 // aka D
#define FEATHER_HSPI_CLK 14  // aka SCK
#define FEATHER_HSPI_CS 15   // chip select

#define LCD_SPI_HOST FEATHER_HSPI // this also changes all SPI(...) invocations! handy innit

#define SPI(...) SPI_LITERAL(LCD_SPI_HOST, __VA_ARGS__)
#define SPI_LITERAL(HOST, ...) SPI_LITERAL_LITERAL(HOST, __VA_ARGS__)
#define SPI_LITERAL_LITERAL(HOST, ...) HOST##_##__VA_ARGS__

// MOSI, MISO, SCLK, & CS are covered in `spi.h` (above)
#define LCD_RST FEATHER_27
#define LCD_DC FEATHER_33
#define LCD_BACKLIGHT FEATHER_32

#endif // PINS_H
