#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "big-ascii.h"
#include "circlebuffer.h"
#include "graphics.h"
#include "lcd.h"

#include <stdint.h>

#define BACKGROUND RED // TODO: back to `WHITE`
#define PULSELINE RED
#define PULSEMEAN BLUE
#define PULSEPEAK GREEN
#define MIDPOINT 31
#define NDOTTED 4
#define HZMAX 159
#define VTMAX 127
#define PADDING 3

void init_graphics(void) {
  lcd_init();

  lcd_set_screen(BACKGROUND);
  big_heart(HZMAX - PADDING, PADDING, RED);
  big_B(HZMAX - PADDING, PADDING + 4 * (BIGASCII_W + 1), RED);
  big_P(HZMAX - PADDING, PADDING + 5 * (BIGASCII_W + 1), RED);
  big_M(HZMAX - PADDING, PADDING + 6 * (BIGASCII_W + 1), RED);
}

uint8_t is_heartbeat(uint16_t v) {
  static int8_t hist[128];
  static uint16_t runmean = 1024;
  static int16_t runpeak = 0;
  static uint8_t peakidx = 0;
  static uint8_t dotted = 0;
  static uint8_t falling = 0; // if we recorded a heartbeat & are now waiting for this "bump" to finish

  // Inlined part of `lcd_block` for a dotted line:
  LCD_TRUST_SET_ADDR(MIDPOINT, 0, MIDPOINT, 127);
  for (uint16_t ij = 0; ij != 128; ++ij) {
    spill_send_8b_data(((uint16_t)(!dotted)) - 1);
    if ((++dotted) == NDOTTED) { dotted = 0; }
  }
  if ((++dotted) == NDOTTED) { dotted = 0; }

  uint8_t tgt;
  uint8_t pen = (((uint8_t)(128U ^ hist[0])) >> 2);
  for (uint8_t i = 0; i != 127; ++i) {
    tgt = ((uint8_t)(128U ^ (hist[i] = hist[i + 1]))) >> 2;
    if (pen == tgt) {
      lcd_look_to_the_cookie(pen, i, tgt, PULSELINE, BACKGROUND);
    } else if (pen > tgt) {
      lcd_look_to_the_cookie(tgt, i, pen - 1, PULSELINE, BACKGROUND);
    } else {
      lcd_look_to_the_cookie(pen + 1, i, tgt, PULSELINE, BACKGROUND);
    }
    pen = tgt;
  }
  // lcd_pixel(((uint8_t)(128U ^ hist[127])) >> 2, 127, BACKGROUND);
  {
    int16_t naive_sum = v - (runmean >> 1);
    hist[127] = (naive_sum < -128) ? -128 : ((naive_sum > 127 ? 127 : naive_sum));
  }
  // lcd_pixel(((uint8_t)(128U ^ hist[127])) >> 2, 127, PULSELINE);
  tgt = ((uint8_t)(128U ^ hist[127])) >> 2;
  if (pen == tgt) {
    lcd_block(pen, 127, tgt, 127, PULSELINE);
  } else if (pen > tgt) {
    lcd_block(tgt, 127, pen - 1, 127, PULSELINE);
  } else {
    lcd_block(pen + 1, 127, tgt, 127, PULSELINE);
  }

  if ((v << 1) > runmean) {
    ++runmean;
  } else if ((v << 1) < runmean) {
    falling = 0;
    GPIO_PULL_LO(PIN_BUZZER);
    --runmean;
  }
  if ((hist[127] << 3) > runpeak) {
    lcd_block(((uint8_t)(128U ^ (runpeak >> 3))) >> 2, 0, ((uint8_t)(128U ^ (runpeak >> 3))) >> 2, 127, BACKGROUND); // Erase the last peak
    runpeak = (hist[127] << 3);
    peakidx = 127;
  } else if (peakidx) {
    --peakidx;
  } else {
    // Iterate & find the new peak (unfortunately)
    lcd_block(((uint8_t)(128U ^ (runpeak >> 3))) >> 2, 0, ((uint8_t)(128U ^ (runpeak >> 3))) >> 2, 127, BACKGROUND); // Erase the last peak
    runpeak = (hist[0] << 3);
    peakidx = 0;
    for (uint8_t i = 1; i != 128; ++i) {
      if ((hist[i] << 3) >= runpeak) {
        runpeak = (hist[i] << 3);
        peakidx = i;
      }
    }
  }
  lcd_block(((uint8_t)(128U ^ (runpeak >> 3))) >> 2, 0, ((uint8_t)(128U ^ (runpeak >> 3))) >> 2, 127, PULSEPEAK);

  if ((!falling) && ((hist[127] << 4) > runpeak)) { // NOTE <<4 NOT <<3: only testing for > half-peak
    falling = 1;
    GPIO_PULL_HI(PIN_BUZZER);
    return 1;
  }
  return 0;
}

void update_bpm(uint16_t /* just in a hell of a case */ bpm) {
  lcd_block(HZMAX - PADDING - BIGASCII_H + 1, PADDING + BIGASCII_W, HZMAX - PADDING, PADDING + 4 * (BIGASCII_W + 1) - 1, BACKGROUND);
  if (bpm < 1000) {
    if (bpm < 100) {
      if (bpm < 10) {
        big_integer(bpm, HZMAX - PADDING, PADDING + 3 * (BIGASCII_W + 1), RED);
      } else {
        big_integer(bpm / 10U, HZMAX - PADDING, PADDING + 2 * (BIGASCII_W + 1), RED);
        big_integer(bpm % 10U, HZMAX - PADDING, PADDING + 3 * (BIGASCII_W + 1), RED);
      }
    } else {
      big_integer(bpm / 100U, HZMAX - PADDING, PADDING + BIGASCII_W + 1, RED);
      big_integer((bpm / 10U) % 10U, HZMAX - PADDING, PADDING + 2 * (BIGASCII_W + 1), RED);
      big_integer(bpm % 10U, HZMAX - PADDING, PADDING + 3 * (BIGASCII_W + 1), RED);
    }
  } else {
    big_question(HZMAX - PADDING, PADDING + BIGASCII_W + 1, RED);
    big_question(HZMAX - PADDING, PADDING + 2 * (BIGASCII_W + 1), RED);
    big_question(HZMAX - PADDING, PADDING + 3 * (BIGASCII_W + 1), RED);
  }
}

#endif // GRAPHICS_H
