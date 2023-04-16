#ifndef LCD_SPI_H
#define LCD_SPI_H

// Adafruit 358 + ST7735R
// https://www.adafruit.com/product/358
// https://cdn-shop.adafruit.com/datasheets/ST7735R_V0.2.pdf

#include "gpio.h"
#include "pins.h"
#include "spill.h"
#include "st7735.h"

#include <rom/ets_sys.h>

#ifndef NDEBUG
static uint8_t LCD_INITIALIZED = 0;
#endif // NDEBUG

#define LCD_TRUST_SEND_COMMAND(COMMAND, NBYTE, DELAY, ...)                                                         \
  do {                                                                                                             \
    _Static_assert((NBYTE) == (sizeof((uint8_t[]){__VA_ARGS__})));                                                 \
    _Static_assert((NBYTE) == (uint8_t)(NBYTE));                                                                   \
    assert(LCD_INITIALIZED);                                                                                       \
    assert(SPILL_IS_OPEN);                                                                                         \
    GPIO_PULL_LO(LCD_DC); /* indicates an incoming command */                                                      \
    spill_send_8b(COMMAND);                                                                                        \
    GPIO_PULL_HI(LCD_DC); /* end of a command */                                                                   \
    spill_send_8b(sizeof((uint8_t[]){__VA_ARGS__}));                                                               \
    __VA_OPT__(_Pragma("GCC unroll 256") for (uint8_t i = 0; i < (uint8_t)sizeof((uint8_t[]){__VA_ARGS__}); ++i) { \
      spill_send_8b(((uint8_t[]){__VA_ARGS__})[i]);                                                                \
    })                                                                                                             \
    if (DELAY) { ets_delay_us((((DELAY) == 255) ? 500 : DELAY) * 1000U); }                                         \
  } while (0)

__attribute__((always_inline)) inline static void lcd_init(void) {
  assert(!LCD_INITIALIZED);
  assert(!SPILL_IS_SET_UP); // done below
#ifndef NDEBUG
  LCD_INITIALIZED = 1;
#endif // NDEBUG

  spill_setup();

  GPIO_ENABLE_OUTPUT(LCD_RST);
  GPIO_ENABLE_OUTPUT(LCD_DC);
  GPIO_ENABLE_OUTPUT(LCD_BACKLIGHT);

  // Turn off the display
  GPIO_PULL_LO(LCD_BACKLIGHT);

  // Reset the ST7735
  GPIO_PULL_HI(LCD_RST); // it looks like (in the Lab 4 starter code `lcd_pin_init`) we never turn this off...?
  // vTaskDelay(100 / portTICK_PERIOD_MS);
  // GPIO_PULL_LO(LCD_RST);
  // vTaskDelay(100 / portTICK_PERIOD_MS);

  // Initialize (from ESP-IDF example)
  // while (lcd_init_cmds[cmd].databytes != 0xff) {
  //   lcd_cmd(spi, lcd_init_cmds[cmd].cmd, false);
  //   lcd_data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F);
  //   if (lcd_init_cmds[cmd].databytes & 0x80) {
  //     vTaskDelay(100 / portTICK_PERIOD_MS);
  //   }
  //   cmd++;
  // }

  // SPI command sequence from ESE350 ST7735 library (almost verbatim):
  spill_open();
  // Software reset. This first one is needed because of the RC reset.
  SPILL_SAFE_COMMAND(CMD_SWRESET, 0, 150);
  // Exit sleep mode
  SPILL_SAFE_COMMAND(CMD_SLPOUT, 0, 255);
  // Frame rate control 1
  SPILL_SAFE_COMMAND(CMD_FRMCTR1, 3, 0, 0x01, 0x2C, 0x2D);
  // Frame rate control 2
  SPILL_SAFE_COMMAND(CMD_FRMCTR2, 3, 0, 0x01, 0x2C, 0x2D);
  // Frame rate control 3
  SPILL_SAFE_COMMAND(CMD_FRMCTR3, 6, 0, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D);
  // Display inversion
  SPILL_SAFE_COMMAND(CMD_INVCTR, 1, 0, 0x07);
  // Power control 1
  SPILL_SAFE_COMMAND(CMD_PWCTR1, 3, 5, 0x0A, 0x02, 0x84);
  // Power control 2
  SPILL_SAFE_COMMAND(CMD_PWCTR2, 1, 5, 0xC5);
  // Power control 3
  SPILL_SAFE_COMMAND(CMD_PWCTR3, 2, 5, 0x0A, 0x00);
  // Power control 4
  SPILL_SAFE_COMMAND(CMD_PWCTR4, 2, 5, 0x8A, 0x2A);
  // Power control 5
  SPILL_SAFE_COMMAND(CMD_PWCTR5, 2, 5, 0x8A, 0xEE);
  // Vcom control 1
  SPILL_SAFE_COMMAND(CMD_VMCTR1, 1, 0, 0x0E);
  // Inversion off
  SPILL_SAFE_COMMAND(CMD_INVOFF, 0, 0);
  // Memory Access control
  SPILL_SAFE_COMMAND(CMD_MADCTL, 1, 0, 0xC8);
  // Interface pixel format
  SPILL_SAFE_COMMAND(CMD_COLMOD, 1, 0, 0x05);
  // Column
  SPILL_SAFE_COMMAND(CMD_CASET, 4, 0, 0x00, 0x00, 0x00, 0x7F);
  // Page
  SPILL_SAFE_COMMAND(CMD_RASET, 4, 0, 0x00, 0x00, 0x00, 0x9F);
  // Positive Gamma
  SPILL_SAFE_COMMAND(CMD_GMCTRP1, 16, 0, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10);
  // Negative Gamma
  SPILL_SAFE_COMMAND(CMD_GMCTRN1, 16, 0, 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10);
  // Normal display on
  SPILL_SAFE_COMMAND(CMD_NORON, 0, 10);
  // Set display on
  SPILL_SAFE_COMMAND(CMD_DISPON, 0, 100);
  // Default to rotation 3
  SPILL_SAFE_COMMAND(CMD_MADCTL, 1, 10, MADCTL_MX | MADCTL_MV | MADCTL_RGB);
  spill_close();

  // TODO: fill background HERE

  // Turn on the display
  GPIO_PULL_HI(LCD_BACKLIGHT);
}

#define LCD_TRUST_SET_ADDR(X0, Y0, X1, Y1)             \
  do {                                                 \
    assert(LCD_INITIALIZED);                           \
    assert(SPILL_IS_OPEN);                             \
    SPILL_SAFE_COMMAND(CMD_CASET, 4, 0, 0, X0, 0, X1); \
    SPILL_SAFE_COMMAND(CMD_RASET, 4, 0, 0, Y0, 0, Y1); \
    SPILL_SAFE_COMMAND(CMD_RAMWR, 0, 5);               \
  } while (0)

#endif // LCD_SPI_H
