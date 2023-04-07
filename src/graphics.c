#include "circlebuffer.h"
#include "lcd.h"
#include "st7735.h"

#define BACKGROUND RGB565(255, 255, 255)
#define PULSELINE RGB565(255, 0, 0)
#define PULSEMEAN RGB565(0, 0, 255)
#define PULSEPEAK RGB565(0, 255, 0)

void init_graphics(void) {
  lcd_init();
  lcd_set_screen(BACKGROUND);
}

void vis_pulse(uint16_t v) {
  static uint8_t hist[128];
  static uint16_t runmean = 512;
  static uint16_t runpeak = 1023;

  for (uint8_t i = 0; i != 127; ++i) {
    lcd_draw_pixel(hist[i] >> 2, i, BACKGROUND); // Erase the last one
    hist[i] = hist[i + 1];
    lcd_draw_pixel(hist[i] >> 2, i, PULSELINE); // Draw the new one
  }
  lcd_draw_pixel(hist[127] >> 2, 127, BACKGROUND);
  lcd_draw_pixel((hist[127] = (v >> 2)) >> 2, 127, PULSELINE);

  lcd_draw_block(runmean >> 5, 0, runmean >> 5, 127, BACKGROUND); // Erase the last mean
  lcd_draw_block(runpeak >> 5, 0, runpeak >> 5, 127, BACKGROUND); // Erase the last peak
  if ((v << 1) > runmean) {
    ++runmean;
  } else if ((v << 1) < runmean) {
    --runmean;
  }
  if ((v << 1) > runpeak) {
    runpeak = (v << 1);
  } else {
    --runpeak;
  }
  lcd_draw_block(runmean >> 5, 0, runmean >> 5, 127, PULSEMEAN);
  lcd_draw_block(runpeak >> 5, 0, runpeak >> 5, 127, PULSEPEAK);
}
