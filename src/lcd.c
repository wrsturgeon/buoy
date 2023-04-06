#include "lcd.h"
#include "ascii.h"
#include "st7735.h"

__attribute__((always_inline)) void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color) {
  LCD_setAddr(x, y, x, y);
  SPI_ControllerTx_16bit(color);
}

void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor) {
  uint16_t row = character - 0x20; // Determine row of ASCII table starting at space
  unsigned i, j;
  //	if ((LCD_WIDTH-x>7)&&(LCD_HEIGHT-y>7)){
  for (i = 0; i < 5; i++) {
    uint8_t pixels = ASCII[row][i]; // Go through the list of pixels
    for (j = 0; j < 8; j++) {
      if (((pixels >> j) & 1) == 1) {
        LCD_drawPixel(x + i, y + j, fColor);
      } else {
        LCD_drawPixel(x + i, y + j, bColor);
      }
    }
  }
  //	}
}

__attribute__((always_inline))
uint8_t
sq_dist_test(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t rsq) {
  uint8_t dx = x1 - x0;
  uint8_t dy = y1 - y0;
  return ((uint16_t)((((uint16_t)dx) * (uint16_t)dx) + (((uint16_t)dy) * (uint16_t)dy))) < rsq;
}

// NOLINTNEXTLINE(misc-no-recursion)
void circle_remainder_recursive(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t rsq, uint16_t color) {
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

  LCD_drawBlock(x1, y1, x1 + l, y1 + l, color);
  LCD_drawBlock(x1, y0 - dy - l, x1 + l, y0 - dy, color);
  LCD_drawBlock(x0 - dx - l, y1, x0 - dx, y1 + l, color);
  LCD_drawBlock(x0 - dx - l, y0 - dy - l, x0 - dx, y0 - dy, color);
  LCD_drawBlock(x0 + dy, y0 + dx, x0 + dy + l, y0 + dx + l, color);
  LCD_drawBlock(x0 + dy, y0 - dx - l, x0 + dy + l, y0 - dx, color);
  LCD_drawBlock(x0 - dy - l, y0 + dx, x0 - dy, y0 + dx + l, color);
  LCD_drawBlock(x0 - dy - l, y0 - dx - l, x0 - dy, y0 - dx, color);

  ++l;
  circle_remainder_recursive(x0, y0, x1, y1 + l, rsq, color);
  circle_remainder_recursive(x0, y0, x1 + l, y1, rsq, color);
}

__attribute__((always_inline)) void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color) {
  // Draw the biggest rectangle you can, recursively, down to 1 pixel
  // sqrt(1/2)*256=181 so x*sqrt(1/2) ~= (x*181)>>8 without float mul
  uint8_t hsl /* half side-length */ = (uint16_t)(181 * (uint16_t)radius) >> 8;
  uint8_t y1 = y0 + hsl;
  LCD_drawBlock(x0 - hsl, y0 - hsl, x0 + hsl, y1, color);
  uint16_t rsq = (uint16_t)(((uint16_t)radius) * (uint16_t)radius);
  circle_remainder_recursive(x0, y0, x0, y1, rsq, color);
}

void LCD_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t c) {
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
    LCD_drawPixel(*px, *py, c);
    if (((e += *dz) << 1) >= *di) {
      e -= *di;
      z += zncr;
    }
  }
}

__attribute__((always_inline)) void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
  //  for (uint8_t y = y0; y < y1; ++y) {
  //    for (uint8_t x = x0; x < x1; ++x) {
  //      LCD_drawPixel(x, y, color);
  //    }
  //  }
  sendCommands((uint8_t[]){
                   ST7735_CASET, 4, // Column (4 arguments)
                   0U,              // start (upper 8 bits)
                   x0,              // start (lower 8 bits)
                   0U,              // end (upper 8 bits)
                   x1,              // end (lower 8 bits)
                   0U,              // delay before next command
                   ST7735_RASET,
                   4,  // Page (4 arguments)
                   0U, // start (upper 8 bits)
                   y0, // start (lower 8 bits)
                   0U, // end (upper 8 bits)
                   y1, // end (lower 8 bits)
                   0U, // delay
                   ST7735_RAMWR,
                   0,
                   /*5*/ 0 // write to RAM, delay 5 clocks(?)
               },
               3);
  clear(LCD_PORT, LCD_TFT_CS); // pull CS low to start communication
  uint8_t upper_color = (color >> 8);
  uint8_t lower_color = color; // not sure if this actually speeds anything up
  for (uint16_t ij = 0; ij < (uint16_t)(((uint16_t)(y1 - y0 + 1)) * (uint16_t)(x1 - x0 + 1)); ++ij) {
    SPDR = upper_color; // Place data to be sent on registers
    do
      ;
    while (!(SPSR & (1U << SPIF))); // wait for end of transmission
    SPDR = lower_color;             // Place data to be sent on registers
    do
      ;
    while (!(SPSR & (1U << SPIF))); // wait for end of transmission
  }
  set(LCD_PORT, LCD_TFT_CS); // pull CS high to end communication
}

__attribute__((always_inline)) void LCD_setScreen(uint16_t color) {
  LCD_drawBlock(0, 0, 159, 127, color);
}

__attribute__((always_inline)) void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg) {
  if (!str) { return; }
  char c;
  while ((c = (*(str++)))) {
    // this doesn't have to be super performant as long as we only call it when necessary
    LCD_drawChar(x, y, c, fg, bg);
    x += 6;
  }
}
