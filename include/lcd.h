#ifndef BUOY_INCLUDE_LCD_H
#define BUOY_INCLUDE_LCD_H

#include <stdint.h>

// Converts 24b RGB to 16b 5-6-5 RGB (respectively) with some bit trickery
#define RGB565(R, G, B) (((uint16_t)(((uint16_t)(((uint8_t)(R)) & 0b11111000U)) << 8U)) | ((uint16_t)(((uint16_t)(((uint8_t)(G)) & 0b11111100U)) << 5U)) | (((uint8_t)(B)) >> 3U))
// Original: #define RGB565(R,G,B) ((((31*((R)+4))/255)<<11) | (((63*((G)+2))/255)<<5) | ((31*((B)+4))/255))

void lcd_draw_pixel(uint8_t x, uint8_t y, uint16_t color);
void lcd_draw_circle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color);
void lcd_draw_block(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void lcd_set_screen(uint16_t color);
void lcd_draw_string(uint8_t x, uint8_t y, char const* str, uint16_t fg, uint16_t bg);

void lcd_draw_char(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor);
void lcd_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t c);

#endif // BUOY_INCLUDE_LCD_H
