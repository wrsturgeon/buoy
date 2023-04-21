#ifndef LCD_H
#define LCD_H

#include "ascii.h"
#include "bitbang/st7735.h"
#include "sane-assert.h"

#include <stdint.h>

// Converts 24b RGB to 16b 5-6-5 RGB (respectively) with some bit trickery
// #define RGB565(R,G,B) ((((31*((R)+4))/255)<<11) | (((63*((G)+2))/255)<<5) | ((31*((B)+4))/255))
#define RGB565(R, G, B) ((((R) >> 3U) << 11U) | (((G) >> 2U) << 5U) | ((B) >> 3U))

#define RED RGB565(255, 0, 0)
#define GREEN RGB565(0, 255, 0)
#define BLUE RGB565(0, 0, 255)
#define WHITE RGB565(255, 255, 255)
#define BLACK RGB565(0, 0, 0)

#define LCD_TRUST_SET_ADDR(X0, Y0, X1, Y1)      \
  do {                                          \
    SANE_ASSERT(ST7735_READY);                  \
    SANE_ASSERT(SPI_IS_OPEN);                   \
    SPI_COMMAND(CMD_CASET, 4, 0, 0, X0, 0, X1); \
    SPI_COMMAND(CMD_RASET, 4, 0, 0, Y0, 0, Y1); \
    SPI_COMMAND(CMD_RAMWR, 0, 5);               \
  } while (0)

static void lcd_pixel(uint8_t x, uint8_t y, uint16_t color) {
  SANE_ASSERT(ST7735_READY);
  SANE_ASSERT(!SPI_IS_OPEN);
  spi_open();
  LCD_TRUST_SET_ADDR(x, y, x, y);
  spi_send_16b(color);
  spi_close();
}

static void lcd_char(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor) {
  SANE_ASSERT(ST7735_READY);
  SANE_ASSERT(!SPI_IS_OPEN);
  uint16_t row = character - 0x20; // Determine row of ASCII table starting at space
  unsigned i, j;
  for (i = 0; i < 5; i++) {
    uint8_t pixels = ASCII[row][i]; // Go through the list of pixels
    for (j = 0; j < 8; j++) {
      if (((pixels >> j) & 1) == 1) {
        lcd_pixel(x + i, y + j, fColor);
      } else {
        lcd_pixel(x + i, y + j, bColor);
      }
    }
  }
}

static void lcd_block(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
  SANE_ASSERT(ST7735_READY);
  SANE_ASSERT(!SPI_IS_OPEN);
  spi_open();
  LCD_TRUST_SET_ADDR(x0, y0, x1, y1);
  for (uint16_t ij = 0; ij != (uint16_t)(((uint16_t)(y1 - y0 + 1)) * (uint16_t)(x1 - x0 + 1)); ++ij) { spi_send_16b(color); }
  spi_close();
}

__attribute__((always_inline)) inline static uint8_t sq_dist_test(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t rsq) {
  uint8_t dx = x1 - x0;
  uint8_t dy = y1 - y0;
  return ((uint16_t)((((uint16_t)dx) * (uint16_t)dx) + (((uint16_t)dy) * (uint16_t)dy))) < rsq;
}

// NOLINTNEXTLINE(misc-no-recursion)
static void circle_remainder_recursive(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t rsq, uint16_t color) {
  // recursively draw the biggest square you can
  // find l s.t. (x1+l,y1+l) is exactly `radius` away from (x0,y0),
  // and draw that square 8 times around the cicle
  if (!sq_dist_test(x0, y0, x1, y1, rsq)) {
    return;
  }              // base case: already out of bounds
  uint8_t l = 1; // binary search
  while (sq_dist_test(x0, y0, x1 + l, y1 + l, rsq)) {
    l <<= 1;
  }
  uint8_t dl;
  l -= (dl = (l >> 1)); // walking back
  while (dl >>= 1) {
    l += dl;
    if (!sq_dist_test(x0, y0, x1 + l, y1 + l, rsq)) {
      l -= dl;
    }
  }
  // necessary to do (x0 - (x1 - x0)) instead of ((x0 << 1) - x1) b/c overflow
  uint8_t dx = x1 - x0;
  uint8_t dy = y1 - y0;
  --l;

  lcd_block(x1, y1, x1 + l, y1 + l, color);
  lcd_block(x1, y0 - dy - l, x1 + l, y0 - dy, color);
  lcd_block(x0 - dx - l, y1, x0 - dx, y1 + l, color);
  lcd_block(x0 - dx - l, y0 - dy - l, x0 - dx, y0 - dy, color);
  lcd_block(x0 + dy, y0 + dx, x0 + dy + l, y0 + dx + l, color);
  lcd_block(x0 + dy, y0 - dx - l, x0 + dy + l, y0 - dx, color);
  lcd_block(x0 - dy - l, y0 + dx, x0 - dy, y0 + dx + l, color);
  lcd_block(x0 - dy - l, y0 - dx - l, x0 - dy, y0 - dx, color);

  ++l;
  circle_remainder_recursive(x0, y0, x1, y1 + l, rsq, color);
  circle_remainder_recursive(x0, y0, x1 + l, y1, rsq, color);
}

__attribute__((always_inline)) inline static void lcd_circle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color) {
  SANE_ASSERT(ST7735_READY);
  SANE_ASSERT(!SPI_IS_OPEN);
  // Draw the biggest rectangle you can, recursively, down to 1 pixel
  // sqrt(1/2)*256=181 so x*sqrt(1/2) ~= (x*181)>>8 without float mul
  uint8_t hsl /* half side-length */ = (uint16_t)(181 * (uint16_t)radius) >> 8;
  uint8_t y1 = y0 + hsl;
  lcd_block(x0 - hsl, y0 - hsl, x0 + hsl, y1, color);
  uint16_t rsq = (uint16_t)(((uint16_t)radius) * (uint16_t)radius);
  circle_remainder_recursive(x0, y0, x0, y1, rsq, color);
}

/*

static void lcd_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t c) {
  SANE_ASSERT(ST7735_READY);
  SANE_ASSERT(!SPI_IS_OPEN);
  int8_t flipx, flipy, incr, zncr;
  int16_t dx, dy, i, z, e = 0;
  int16_t const *i0, *i1, *px, *py, *di, *dz;
  dx = (flipx = (x1 < x0)) ? x0 - x1 : (x1 - x0);
  dy = (flipy = (y1 < y0)) ? y0 - y1 : (y1 - y0);
  if (dy <= dx) {
    incr = (flipx ? -1 : 1);
    zncr = (flipy ? -1 : 1);
    i0 = &x0;
    i1 = &x1;
    px = &i;
    py = &z;
    di = &dx;
    dz = &dy;
    z = y0;
  } else {
    incr = (flipy ? -1 : 1);
    zncr = (flipx ? -1 : 1);
    i0 = &y0;
    i1 = &y1;
    px = &z;
    py = &i;
    di = &dy;
    dz = &dx;
    z = x0;
  }
  for (i = *i0; i != *i1; i += incr) {
    lcd_pixel(*px, *py, c);
    if (((e += *dz) << 1) >= *di) {
      e -= *di;
      z += zncr;
    }
  }
}

*/

// Vertical line.
__attribute__((always_inline)) inline static void lcd_look_to_the_cookie(uint8_t x0, uint8_t y0, uint8_t x1, uint16_t color1, uint16_t color2) {
  SANE_ASSERT(ST7735_READY);
  SANE_ASSERT(!SPI_IS_OPEN);
  spi_open();
  LCD_TRUST_SET_ADDR(x0, y0, x1, y0 + 1);
  for (uint16_t ij = 0; ij != (uint16_t)(x1 - x0 + 1); ++ij) { spi_send_16b(color1); }
  for (uint16_t ij = 0; ij != (uint16_t)(x1 - x0 + 1); ++ij) { spi_send_16b(color2); }
  spi_close();
}

__attribute__((always_inline)) inline static void lcd_set_screen(uint16_t color) {
  SANE_ASSERT(ST7735_READY);
  lcd_block(0, 0, 159, 127, color);
}

__attribute__((always_inline)) inline static void lcd_string(uint8_t x, uint8_t y, char const* str, uint16_t fg, uint16_t bg) {
  SANE_ASSERT(ST7735_READY);
  SANE_ASSERT(!SPI_IS_OPEN);
  if (!str) { return; }
  char c;
  while ((c = (*(str++)))) {
    // this doesn't have to be super performant as long as we only call it when necessary
    lcd_char(x, y, c, fg, bg);
    x += 6;
  }
}

#endif // LCD_H
