#ifndef BUOY_INCLUDE_LCD_H
#define BUOY_INCLUDE_LCD_H

#include <stdint.h>

// Converts 24b RGB to 16b 5-6-5 RGB (respectively) with some bit trickery
#define RGB565(R, G, B) (((uint16_t)(((uint16_t)((R)&0b11111000)) << 8)) | ((uint16_t)(((uint16_t)((G)&0b11111100)) << 5)) | ((B) >> 3))
// Original: #define RGB565(R,G,B) ((((31*((R)+4))/255)<<11) | (((63*((G)+2))/255)<<5) | ((31*((B)+4))/255))

void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor);
void LCD_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t c);

__attribute__((always_inline)) void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color);
__attribute__((always_inline)) void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
__attribute__((always_inline)) void LCD_setScreen(uint16_t color);
__attribute__((always_inline)) void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg);

#endif // BUOY_INCLUDE_LCD_H
