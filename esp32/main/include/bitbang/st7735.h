#ifndef ST7735_H
// #define ST7735_H // actually redefined below as height lol

#include "bitbang/spi.h"
#include "sane-assert.h"

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

#define CMD_NOP (0x00U)
#define CMD_SWRESET (0x01U)
#define CMD_RDDID (0x04U)
#define CMD_RDDST (0x09U)
#define CMD_SLPIN (0x10U)
#define CMD_SLPOUT (0x11U)
#define CMD_PTLON (0x12U)
#define CMD_NORON (0x13U)
#define CMD_INVOFF (0x20U)
#define CMD_INVON (0x21U)
#define CMD_DISPOFF (0x28U)
#define CMD_DISPON (0x29U)
#define CMD_CASET (0x2AU)
#define CMD_RASET (0x2BU)
#define CMD_RAMWR (0x2CU)
#define CMD_RAMRD (0x2EU)
#define CMD_PTLAR (0x30U)
#define CMD_COLMOD (0x3AU)
#define CMD_MADCTL (0x36U)
#define CMD_FRMCTR1 (0xB1U)
#define CMD_FRMCTR2 (0xB2U)
#define CMD_FRMCTR3 (0xB3U)
#define CMD_INVCTR (0xB4U)
#define CMD_DISSET5 (0xB6U)
#define CMD_PWCTR1 (0xC0U)
#define CMD_PWCTR2 (0xC1U)
#define CMD_PWCTR3 (0xC2U)
#define CMD_PWCTR4 (0xC3U)
#define CMD_PWCTR5 (0xC4U)
#define CMD_VMCTR1 (0xC5U)
#define CMD_RDID1 (0xDAU)
#define CMD_RDID2 (0xDBU)
#define CMD_RDID3 (0xDCU)
#define CMD_RDID4 (0xDDU)
#define CMD_PWCTR6 (0xFCU)
#define CMD_GMCTRP1 (0xE0U)
#define CMD_GMCTRN1 (0xE1U)

#define MADCTL_MY (0x80U)
#define MADCTL_MX (0x40U)
#define MADCTL_MV (0x20U)
#define MADCTL_ML (0x10U)
#define MADCTL_RGB (0x00U)
#define MADCTL_BGR (0x08U)
#define MADCTL_MH (0x04U)

#define ST7735_W (160U)
#define ST7735_H (128U)
#define ST7735_AREA (ST7735_W * ST7735_H)

#define PIN_GND FEATHER_13
#define PIN_VIN FEATHER_12
#define PIN_RST FEATHER_27
#define PIN_TDC FEATHER_33
#define PIN_LIT FEATHER_A0

#ifndef NDEBUG
static uint8_t ST7735_READY = 0;
#endif // NDEBUG

__attribute__((always_inline)) inline static void st7735_init(void) {
  SANE_ASSERT(!SPI_READY);
  SANE_ASSERT(!ST7735_READY);

  spi_init();

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
  ets_delay_us(100000);   // 0.1s
  GPIO_PULL(PIN_RST, HI);

  // SPI command sequence from ESE350 ST7735 library (almost verbatim):
  spi_open();
  SPI_COMMAND(CMD_SWRESET, 0, 150);                                                                                                // Software reset. This first one is needed because of the RC reset.
  SPI_COMMAND(CMD_SLPOUT, 0, 500);                                                                                                 // Exit sleep mode
  SPI_COMMAND(CMD_FRMCTR1, 3, 0, 0x01, 0x2C, 0x2D);                                                                                // Frame rate control 1
  SPI_COMMAND(CMD_FRMCTR2, 3, 0, 0x01, 0x2C, 0x2D);                                                                                // Frame rate control 2
  SPI_COMMAND(CMD_FRMCTR3, 6, 0, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D);                                                              // Frame rate control 3
  SPI_COMMAND(CMD_INVCTR, 1, 0, 0x07);                                                                                             // Display inversion
  SPI_COMMAND(CMD_PWCTR1, 3, 5, 0x0A, 0x02, 0x84);                                                                                 // Power control 1
  SPI_COMMAND(CMD_PWCTR2, 1, 5, 0xC5);                                                                                             // Power control 2
  SPI_COMMAND(CMD_PWCTR3, 2, 5, 0x0A, 0x00);                                                                                       // Power control 3
  SPI_COMMAND(CMD_PWCTR4, 2, 5, 0x8A, 0x2A);                                                                                       // Power control 4
  SPI_COMMAND(CMD_PWCTR5, 2, 5, 0x8A, 0xEE);                                                                                       // Power control 5
  SPI_COMMAND(CMD_VMCTR1, 1, 0, 0x0E);                                                                                             // Vcom control 1
  SPI_COMMAND(CMD_INVOFF, 0, 0);                                                                                                   // Inversion off
  SPI_COMMAND(CMD_MADCTL, 1, 0, 0xC8);                                                                                             // Memory Access control
  SPI_COMMAND(CMD_COLMOD, 1, 0, 0x05);                                                                                             // Interface pixel format
  SPI_COMMAND(CMD_CASET, 4, 0, 0x00, 0x00, 0x00, 0x7F);                                                                            // Column
  SPI_COMMAND(CMD_RASET, 4, 0, 0x00, 0x00, 0x00, 0x9F);                                                                            // Page
  SPI_COMMAND(CMD_GMCTRP1, 16, 0, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10); // Positive Gamma
  SPI_COMMAND(CMD_GMCTRN1, 16, 0, 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10); // Negative Gamma
  SPI_COMMAND(CMD_NORON, 0, 10);                                                                                                   // Normal display on
  SPI_COMMAND(CMD_DISPON, 0, 100);                                                                                                 // Set display on
  SPI_COMMAND(CMD_MADCTL, 1, 10, MADCTL_MX | MADCTL_MV | MADCTL_RGB);                                                              // Default to rotation 3
  spi_close();

#ifndef NDEBUG
  ST7735_READY = 1;
#endif // NDEBUG
}

#endif // ST7735_H
