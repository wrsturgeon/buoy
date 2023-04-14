#ifndef LCD_H
#define LCD_H

// Adafruit 358 + ST7735R
// https://www.adafruit.com/product/358
// https://cdn-shop.adafruit.com/datasheets/ST7735R_V0.2.pdf

#include "gpio.h"
#include "pins.h"
#include "spi.h"

#include <rom/ets_sys.h>

#define LCDW (160U)
#define LCDH (128U)
#define LCDA (LCDW * LCDH)

#define A358_OK (0U)
#define A358_MISC_ERROR (1U)
#define A358_RESPONSE_TIMEOUT (2U)
#define A358_RESPONSE_BUSY (3U)
#define A358_READ_TIMEOUT (4U)
#define A358_WRITE_TIMEOUT (5U)
#define A358_RESPONSE_CRC_ERROR (6U)
#define A358_READ_CRC_ERROR (7U)
#define A358_WRITE_CRC_ERROR (8U)
#define A358_ILLEGAL_COMMAND (9U)
#define A358_WRITE_ERROR (10U)
#define A358_OUT_OF_BOUNDS (11U)

#define ST7735_NOP (0x00U)
#define ST7735_SWRESET (0x01U)
#define ST7735_RDDID (0x04U)
#define ST7735_RDDST (0x09U)
#define ST7735_SLPIN (0x10U)
#define ST7735_SLPOUT (0x11U)
#define ST7735_PTLON (0x12U)
#define ST7735_NORON (0x13U)
#define ST7735_INVOFF (0x20U)
#define ST7735_INVON (0x21U)
#define ST7735_DISPOFF (0x28U)
#define ST7735_DISPON (0x29U)
#define ST7735_CASET (0x2AU)
#define ST7735_RASET (0x2BU)
#define ST7735_RAMWR (0x2CU)
#define ST7735_RAMRD (0x2EU)
#define ST7735_PTLAR (0x30U)
#define ST7735_COLMOD (0x3AU)
#define ST7735_MADCTL (0x36U)
#define ST7735_FRMCTR1 (0xB1U)
#define ST7735_FRMCTR2 (0xB2U)
#define ST7735_FRMCTR3 (0xB3U)
#define ST7735_INVCTR (0xB4U)
#define ST7735_DISSET5 (0xB6U)
#define ST7735_PWCTR1 (0xC0U)
#define ST7735_PWCTR2 (0xC1U)
#define ST7735_PWCTR3 (0xC2U)
#define ST7735_PWCTR4 (0xC3U)
#define ST7735_PWCTR5 (0xC4U)
#define ST7735_VMCTR1 (0xC5U)
#define ST7735_RDID1 (0xDAU)
#define ST7735_RDID2 (0xDBU)
#define ST7735_RDID3 (0xDCU)
#define ST7735_RDID4 (0xDDU)
#define ST7735_PWCTR6 (0xFCU)
#define ST7735_GMCTRP1 (0xE0U)
#define ST7735_GMCTRN1 (0xE1U)

#define MADCTL_MY 0x80U
#define MADCTL_MX 0x40U
#define MADCTL_MV 0x20U
#define MADCTL_ML 0x10U
#define MADCTL_RGB 0x00U
#define MADCTL_BGR 0x08U
#define MADCTL_MH 0x04U

static uint8_t LCD_INITIALIZED = 0;

#define LCD_TRUST_SEND_COMMAND(COMMAND, NBYTE, DELAY, ...)                                                         \
  do {                                                                                                             \
    _Static_assert((NBYTE) == (sizeof((uint8_t[]){__VA_ARGS__})));                                                 \
    _Static_assert((NBYTE) == (uint8_t)(NBYTE));                                                                   \
    GPIO_PULL_LO(LCD_DC); /* indicates an incoming command */                                                      \
    spi_trust_send_8b(COMMAND);                                                                                    \
    GPIO_PULL_HI(LCD_DC); /* end of a command */                                                                   \
    spi_trust_send_8b(sizeof((uint8_t[]){__VA_ARGS__}));                                                           \
    __VA_OPT__(_Pragma("GCC unroll 256") for (uint8_t i = 0; i < (uint8_t)sizeof((uint8_t[]){__VA_ARGS__}); ++i) { \
      spi_trust_send_8b(((uint8_t[]){__VA_ARGS__})[i]);                                                            \
    })                                                                                                             \
    if (DELAY) { ets_delay_us((((DELAY) == 255) ? 500 : DELAY) * 1000U); }                                         \
  } while (0)

__attribute__((always_inline)) inline static void lcd_init(void) {
#ifndef NDEBUG
  assert(!LCD_INITIALIZED);
  LCD_INITIALIZED = 1;
#endif

  spi_init();

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
  spi_open_comm();
  // Software reset. This first one is needed because of the RC reset.
  LCD_TRUST_SEND_COMMAND(ST7735_SWRESET, 0, 150);
  // Exit sleep mode
  LCD_TRUST_SEND_COMMAND(ST7735_SLPOUT, 0, 255);
  // Frame rate control 1
  LCD_TRUST_SEND_COMMAND(ST7735_FRMCTR1, 3, 0, 0x01, 0x2C, 0x2D);
  // Frame rate control 2
  LCD_TRUST_SEND_COMMAND(ST7735_FRMCTR2, 3, 0, 0x01, 0x2C, 0x2D);
  // Frame rate control 3
  LCD_TRUST_SEND_COMMAND(ST7735_FRMCTR3, 6, 0, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D);
  // Display inversion
  LCD_TRUST_SEND_COMMAND(ST7735_INVCTR, 1, 0, 0x07);
  // Power control 1
  LCD_TRUST_SEND_COMMAND(ST7735_PWCTR1, 3, 5, 0x0A, 0x02, 0x84);
  // Power control 2
  LCD_TRUST_SEND_COMMAND(ST7735_PWCTR2, 1, 5, 0xC5);
  // Power control 3
  LCD_TRUST_SEND_COMMAND(ST7735_PWCTR3, 2, 5, 0x0A, 0x00);
  // Power control 4
  LCD_TRUST_SEND_COMMAND(ST7735_PWCTR4, 2, 5, 0x8A, 0x2A);
  // Power control 5
  LCD_TRUST_SEND_COMMAND(ST7735_PWCTR5, 2, 5, 0x8A, 0xEE);
  // Vcom control 1
  LCD_TRUST_SEND_COMMAND(ST7735_VMCTR1, 1, 0, 0x0E);
  // Inversion off
  LCD_TRUST_SEND_COMMAND(ST7735_INVOFF, 0, 0);
  // Memory Access control
  LCD_TRUST_SEND_COMMAND(ST7735_MADCTL, 1, 0, 0xC8);
  // Interface pixel format
  LCD_TRUST_SEND_COMMAND(ST7735_COLMOD, 1, 0, 0x05);
  // Column
  LCD_TRUST_SEND_COMMAND(ST7735_CASET, 4, 0, 0x00, 0x00, 0x00, 0x7F);
  // Page
  LCD_TRUST_SEND_COMMAND(ST7735_RASET, 4, 0, 0x00, 0x00, 0x00, 0x9F);
  // Positive Gamma
  LCD_TRUST_SEND_COMMAND(ST7735_GMCTRP1, 16, 0, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10);
  // Negative Gamma
  LCD_TRUST_SEND_COMMAND(ST7735_GMCTRN1, 16, 0, 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10);
  // Normal display on
  LCD_TRUST_SEND_COMMAND(ST7735_NORON, 0, 10);
  // Set display on
  LCD_TRUST_SEND_COMMAND(ST7735_DISPON, 0, 100);
  // Default to rotation 3
  LCD_TRUST_SEND_COMMAND(ST7735_MADCTL, 1, 10, MADCTL_MX | MADCTL_MV | MADCTL_RGB);
  spi_close_comm();

  // Turn on the display
  GPIO_PULL_HI(LCD_BACKLIGHT);
}

#define LCD_TRUST_SET_ADDR(X0, Y0, X1, Y1)                    \
  do {                                                        \
    LCD_TRUST_SEND_COMMAND(ST7735_CASET, 4, 0, 0, X0, 0, X1); \
    LCD_TRUST_SEND_COMMAND(ST7735_RASET, 4, 0, 0, Y0, 0, Y1); \
    LCD_TRUST_SEND_COMMAND(ST7735_RAMWR, 0, 5);               \
  } while (0)

#endif // LCD_H
