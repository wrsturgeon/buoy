#ifndef BITBANG_LCD_H
#define BITBANG_LCD_H

#include "bitbang/spi.h"
#include "st7735.h"

#define PIN_GND FEATHER_13
#define PIN_VIN FEATHER_12
#define PIN_RST FEATHER_27
#define PIN_TDC FEATHER_33
#define PIN_LIT FEATHER_A0

static uint8_t LCD_READY = 0;

__attribute__((always_inline)) inline static void lcd_init(void) {
  assert(BITBAND_SPI_READY);
  assert(!LCD_READY);

  GPIO_ENABLE_OUTPUT(PIN_GND);
  GPIO_ENABLE_OUTPUT(PIN_VIN);
  GPIO_ENABLE_OUTPUT(PIN_RST);
  GPIO_ENABLE_OUTPUT(PIN_TDC);
  GPIO_ENABLE_OUTPUT(PIN_LIT);

  GPIO_PULL(PIN_GND, LO);
  GPIO_PULL(PIN_VIN, HI);
  GPIO_PULL(PIN_TDC, HI);
  GPIO_PULL(PIN_LIT, LO); // Dim the lights for stage crew
  GPIO_PULL(PIN_RST, LO); // Reset (active low)
  ets_delay_us(1000000);  // 1s
  GPIO_PULL(PIN_RST, HI);

  // SPI command sequence from ESE350 ST7735 library (almost verbatim):
  spi_open();
  SPI_COMMAND(ST7735_SWRESET, 0, 150);                                                                                                // Software reset. This first one is needed because of the RC reset.
  SPI_COMMAND(ST7735_SLPOUT, 0, 500);                                                                                                 // Exit sleep mode
  SPI_COMMAND(ST7735_FRMCTR1, 3, 0, 0x01, 0x2C, 0x2D);                                                                                // Frame rate control 1
  SPI_COMMAND(ST7735_FRMCTR2, 3, 0, 0x01, 0x2C, 0x2D);                                                                                // Frame rate control 2
  SPI_COMMAND(ST7735_FRMCTR3, 6, 0, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D);                                                              // Frame rate control 3
  SPI_COMMAND(ST7735_INVCTR, 1, 0, 0x07);                                                                                             // Display inversion
  SPI_COMMAND(ST7735_PWCTR1, 3, 5, 0x0A, 0x02, 0x84);                                                                                 // Power control 1
  SPI_COMMAND(ST7735_PWCTR2, 1, 5, 0xC5);                                                                                             // Power control 2
  SPI_COMMAND(ST7735_PWCTR3, 2, 5, 0x0A, 0x00);                                                                                       // Power control 3
  SPI_COMMAND(ST7735_PWCTR4, 2, 5, 0x8A, 0x2A);                                                                                       // Power control 4
  SPI_COMMAND(ST7735_PWCTR5, 2, 5, 0x8A, 0xEE);                                                                                       // Power control 5
  SPI_COMMAND(ST7735_VMCTR1, 1, 0, 0x0E);                                                                                             // Vcom control 1
  SPI_COMMAND(ST7735_INVOFF, 0, 0);                                                                                                   // Inversion off
  SPI_COMMAND(ST7735_MADCTL, 1, 0, 0xC8);                                                                                             // Memory Access control
  SPI_COMMAND(ST7735_COLMOD, 1, 0, 0x05);                                                                                             // Interface pixel format
  SPI_COMMAND(ST7735_CASET, 4, 0, 0x00, 0x00, 0x00, 0x7F);                                                                            // Column
  SPI_COMMAND(ST7735_RASET, 4, 0, 0x00, 0x00, 0x00, 0x9F);                                                                            // Page
  SPI_COMMAND(ST7735_GMCTRP1, 16, 0, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10); // Positive Gamma
  SPI_COMMAND(ST7735_GMCTRN1, 16, 0, 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10); // Negative Gamma
  SPI_COMMAND(ST7735_NORON, 0, 10);                                                                                                   // Normal display on
  SPI_COMMAND(ST7735_DISPON, 0, 100);                                                                                                 // Set display on
  SPI_COMMAND(ST7735_MADCTL, 1, 10, MADCTL_MX | MADCTL_MV | MADCTL_RGB);                                                              // Default to rotation 3
  spi_close();

  GPIO_PULL(PIN_LIT, HI); // Let there be light!

  LCD_READY = 1;
}

#endif // BITBANG_LCD_H
