#ifndef BITBANG_SPI_H
#define BITBANG_SPI_H

#include "hardware/gpio.h"
#include "hardware/pins.h"
#include "sane-assert.h"

#include <rom/ets_sys.h>

#if SCREEN_FLIPPED_OVER_MCU
#define PIN_TCS FEATHER_15
#define PIN_MSI FEATHER_33
#define PIN_SCK FEATHER_27
#define PIN_MSO FEATHER_12
#else // SCREEN_FLIPPED_OVER_MCU
#define PIN_TCS FEATHER_32
#define PIN_MSI FEATHER_14
#define PIN_SCK FEATHER_SCL
#define PIN_MSO FEATHER_SDA
#endif // SCREEN_FLIPPED_OVER_MCU

//%%%%%%%%%%%%%%%%
// Timing specs on the ST7735 datasheet, p. 25:
// https://cdn-shop.adafruit.com/datasheets/ST7735R_V0.2.pdf
// #define PLAY_IT_SAFE
#ifdef PLAY_IT_SAFE
#define POSTCMD_DELAY(...) ets_delay_us(__VA_ARGS__);
#else
#define POSTCMD_DELAY(...)
#endif

#ifndef NDEBUG
static uint8_t SPI_READY = 0;
static uint8_t SPI_IS_OPEN = 0;
#endif // NDEBUG

__attribute__((always_inline)) inline static void spi_init(void) {
  SANE_ASSERT(!SPI_READY);

  GPIO_ENABLE_OUTPUT(PIN_TCS);
  GPIO_ENABLE_OUTPUT(PIN_MSI);
  GPIO_ENABLE_OUTPUT(PIN_SCK);
  GPIO_ENABLE_OUTPUT(PIN_MSO);

  GPIO_PULL(PIN_TCS, HI);
  GPIO_PULL(PIN_MSI, LO);
  GPIO_PULL(PIN_SCK, LO);
  GPIO_PULL(PIN_MSO, LO);

#ifndef NDEBUG
  SPI_READY = 1;
#endif // NDEBUG
}

__attribute__((always_inline)) inline static void spi_open(void) {
  SANE_ASSERT(SPI_READY);
  SANE_ASSERT(!SPI_IS_OPEN);
  SANE_ASSERT(!GPIO_GET(PIN_SCK));
  GPIO_PULL(PIN_TCS, LO);
#ifndef NDEBUG
  SPI_IS_OPEN = 1;
#endif // NDEBUG
}

__attribute__((always_inline)) inline static void spi_close(void) {
  SANE_ASSERT(SPI_READY);
  SANE_ASSERT(SPI_IS_OPEN);
  GPIO_PULL(PIN_TCS, HI);
#ifndef NDEBUG
  SPI_IS_OPEN = 0;
#endif // NDEBUG
}

__attribute__((always_inline)) inline static void spi_send_bit(uint8_t bit) {
  SANE_ASSERT(!GPIO_GET(PIN_SCK));
  bit ? GPIO_PULL(PIN_MSI, HI) : GPIO_PULL(PIN_MSI, LO);
  GPIO_PULL(PIN_SCK, HI);
  // Placing these `SANE_ASSERT`s here should help even out the duty cycle
  SANE_ASSERT(SPI_IS_OPEN);
  SANE_ASSERT(!GPIO_GET(PIN_TCS));
  GPIO_PULL(PIN_SCK, LO);
}

__attribute__((always_inline)) inline static void spi_send_8b(uint8_t msg) {
  spi_send_bit(msg & 0b10000000);
  spi_send_bit(msg & 0b01000000);
  spi_send_bit(msg & 0b00100000);
  spi_send_bit(msg & 0b00010000);
  spi_send_bit(msg & 0b00001000);
  spi_send_bit(msg & 0b00000100);
  spi_send_bit(msg & 0b00000010);
  spi_send_bit(msg & 0b00000001);
}

__attribute__((always_inline)) inline static void spi_send_16b(uint16_t msg) {
  spi_send_8b(msg >> 8U); // MSB
  spi_send_8b(msg);       // LSB
}

// clang-format off
#define SPI_MOSI_0()
#define SPI_MOSI_1(...) spi_send_8b(__VA_ARGS__) // fails on purpose with any # of args other than 1: it's our base case
#define SPI_MOSI_2(a, ...) spi_send_8b(a); SPI_MOSI_1(__VA_ARGS__)
#define SPI_MOSI_3(a, ...) spi_send_8b(a); SPI_MOSI_2(__VA_ARGS__)
#define SPI_MOSI_4(a, ...) spi_send_8b(a); SPI_MOSI_3(__VA_ARGS__)
#define SPI_MOSI_5(a, ...) spi_send_8b(a); SPI_MOSI_4(__VA_ARGS__)
#define SPI_MOSI_6(a, ...) spi_send_8b(a); SPI_MOSI_5(__VA_ARGS__)
#define SPI_MOSI_7(a, ...) spi_send_8b(a); SPI_MOSI_6(__VA_ARGS__)
#define SPI_MOSI_8(a, ...) spi_send_8b(a); SPI_MOSI_7(__VA_ARGS__)
#define SPI_MOSI_9(a, ...) spi_send_8b(a); SPI_MOSI_8(__VA_ARGS__)
#define SPI_MOSI_10(a, ...) spi_send_8b(a); SPI_MOSI_9(__VA_ARGS__)
#define SPI_MOSI_11(a, ...) spi_send_8b(a); SPI_MOSI_10(__VA_ARGS__)
#define SPI_MOSI_12(a, ...) spi_send_8b(a); SPI_MOSI_11(__VA_ARGS__)
#define SPI_MOSI_13(a, ...) spi_send_8b(a); SPI_MOSI_12(__VA_ARGS__)
#define SPI_MOSI_14(a, ...) spi_send_8b(a); SPI_MOSI_13(__VA_ARGS__)
#define SPI_MOSI_15(a, ...) spi_send_8b(a); SPI_MOSI_14(__VA_ARGS__)
#define SPI_MOSI_16(a, ...) spi_send_8b(a); SPI_MOSI_15(__VA_ARGS__)
#define SPI_MOSI_17(a, ...) spi_send_8b(a); SPI_MOSI_16(__VA_ARGS__)
#define SPI_MOSI_18(a, ...) spi_send_8b(a); SPI_MOSI_17(__VA_ARGS__)
#define SPI_MOSI_19(a, ...) spi_send_8b(a); SPI_MOSI_18(__VA_ARGS__)
#define SPI_MOSI_20(a, ...) spi_send_8b(a); SPI_MOSI_19(__VA_ARGS__)
#define SPI_MOSI_21(a, ...) spi_send_8b(a); SPI_MOSI_20(__VA_ARGS__)
#define SPI_MOSI_22(a, ...) spi_send_8b(a); SPI_MOSI_21(__VA_ARGS__)
#define SPI_MOSI_23(a, ...) spi_send_8b(a); SPI_MOSI_22(__VA_ARGS__)
#define SPI_MOSI_24(a, ...) spi_send_8b(a); SPI_MOSI_23(__VA_ARGS__)
#define SPI_MOSI_25(a, ...) spi_send_8b(a); SPI_MOSI_24(__VA_ARGS__)
#define SPI_MOSI_26(a, ...) spi_send_8b(a); SPI_MOSI_25(__VA_ARGS__)
#define SPI_MOSI_27(a, ...) spi_send_8b(a); SPI_MOSI_26(__VA_ARGS__)
#define SPI_MOSI_28(a, ...) spi_send_8b(a); SPI_MOSI_27(__VA_ARGS__)
#define SPI_MOSI_29(a, ...) spi_send_8b(a); SPI_MOSI_28(__VA_ARGS__)
#define SPI_MOSI_30(a, ...) spi_send_8b(a); SPI_MOSI_29(__VA_ARGS__)
#define SPI_MOSI_31(a, ...) spi_send_8b(a); SPI_MOSI_30(__VA_ARGS__)
#define SPI_MOSI_32(a, ...) spi_send_8b(a); SPI_MOSI_31(__VA_ARGS__)
// clang-format on

#define SPI_COMMAND(CMD, ARGC, WAIT, ...) SPI_COMMAND_LITERAL(CMD, ARGC, WAIT, __VA_ARGS__)
#define SPI_COMMAND_LITERAL(CMD, ARGC, WAIT, ...)  \
  do {                                             \
    SANE_ASSERT(SPI_IS_OPEN);                      \
    SANE_ASSERT(!GPIO_GET(PIN_TCS));               \
    SANE_ASSERT(GPIO_GET(PIN_TDC));                \
    GPIO_PULL(PIN_TDC, LO); /* p. 27, first row */ \
    spi_send_8b(CMD);                              \
    GPIO_PULL(PIN_TDC, HI);                        \
    SPI_MOSI_##ARGC(__VA_ARGS__);                  \
    POSTCMD_DELAY(WAIT * 1000U)                    \
  } while (0)

#endif // BITBANG_SPI_H
