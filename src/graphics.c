#include "circlebuffer.h"
#include "lcd.h"
#include "st7735.h"

#define BACKGROUND RGB565(255, 255, 255)
#define PULSELINE RGB565(255, 0, 0)
#define PULSEMEAN RGB565(0, 0, 255)
#define PULSEPEAK RGB565(0, 255, 0)
#define MIDPOINT 31

void init_graphics(void) {
  lcd_init();
  lcd_set_screen(BACKGROUND);
}

void vis_pulse(uint16_t v) {
  static int8_t hist[128];
  static uint16_t runmean = 1024;
  static int16_t runpeak = 0;
  static uint8_t peakidx = 0;

  for (uint8_t i = 0; i != 127; ++i) {
    lcd_draw_pixel((128U ^ hist[i]) >> 2, i, BACKGROUND); // Erase the last one
    hist[i] = hist[i + 1];
    lcd_draw_pixel((128U ^ hist[i]) >> 2, i, PULSELINE); // Draw the new one
  }
  lcd_draw_pixel((128U ^ hist[127]) >> 2, 127, BACKGROUND);
  {
    int16_t naive_sum = v - (runmean >> 1);
    hist[127] = (naive_sum < -128) ? -128 : ((naive_sum > 127 ? 127 : naive_sum));
  }
  lcd_draw_pixel((128U ^ hist[127]) >> 2, 127, PULSELINE);

  lcd_draw_block((1024U ^ runpeak) >> 5, 0, (1024U ^ runpeak) >> 5, 127, BACKGROUND); // Erase the last peak
  if ((v << 1) > runmean) {
    ++runmean;
  } else if ((v << 1) < runmean) {
    --runmean;
  }
  if ((hist[127] << 3) > runpeak) {
    runpeak = (hist[127] << 3);
    peakidx = 127;
  } else if (peakidx) {
    --peakidx;
  } else {
    // unfortunately, iterate & find the new peak
    runpeak = (hist[0] << 3);
    peakidx = 0;
    for (uint8_t i = 1; i != 128; ++i) {
      if ((hist[i] << 3) >= runpeak) {
        runpeak = (hist[i] << 3);
        peakidx = i;
      }
    }
  }
  lcd_draw_block(MIDPOINT, 0, MIDPOINT, 127, PULSEMEAN);
  lcd_draw_block((1024U ^ runpeak) >> 5, 0, (1024U ^ runpeak) >> 5, 127, PULSEPEAK);
}
