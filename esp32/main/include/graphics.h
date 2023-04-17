#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "big-ascii.h"
#include "hardware/adc.h" // for ADC_BIT_WIDTH
#include "lcd.h"
#include "sane-assert.h"

#include <stdint.h>

#define VISIBLE_SETUP 0 // 0: no; 1: yes

#define PIN_BUZZER FEATHER_A1

#define BACKGROUND WHITE
#define PULSELINE RED
#define PULSEMEAN BLUE
#define PULSEPEAK GREEN
#define LOG2_COMPRESS_GRAPH 2
#define MIDPOINT 31
#define NDOTTED 4
#define HZMAX 159
#define VTMAX 127
#define PADDING 3

#ifndef NDEBUG
static uint8_t GRAPHICS_READY = 0;
#endif // NDEBUG

__attribute__((always_inline)) inline static void graphics_init(void) {
  SANE_ASSERT(!GRAPHICS_READY);

  st7735_init(); // leaves backlight off so we can initialize behind the scenes

#if VISIBLE_SETUP
  GPIO_PULL(PIN_LIT, HI);
#endif // VISIBLE_SETUP

  lcd_set_screen(BACKGROUND);
  big_heart(HZMAX - PADDING, PADDING, RED);
  big_B(HZMAX - PADDING, PADDING + 4 * (BIGASCII_W + 1), RED);
  big_P(HZMAX - PADDING, PADDING + 5 * (BIGASCII_W + 1), RED);
  big_M(HZMAX - PADDING, PADDING + 6 * (BIGASCII_W + 1), RED);

  GPIO_PULL(PIN_LIT, HI); // Let there be light!

  GPIO_ENABLE_OUTPUT(PIN_BUZZER);

#ifndef NDEBUG
  GRAPHICS_READY = 1;
#endif // NDEBUG
}

_Static_assert(ADC_BIT_WIDTH > 8);
#define BIT_DISPARITY (ADC_BIT_WIDTH - 8)
#define INCR_DOTTED()                          \
  do {                                         \
    if ((++dotted) == NDOTTED) { dotted = 0; } \
  } while (0)
static uint8_t display_and_check_heartbeat(uint16_t v) {
  static uint8_t hist[128];
  static uint16_t runmean = (1ULL << (ADC_BIT_WIDTH - 1U));
  static uint8_t runpeak = 0;
  static uint8_t peakidx = 0;
  static uint8_t dotted = 0;
  static uint8_t falling = 0; // if we recorded a heartbeat & are now waiting for this "bump" to finish

  // Inlined part of `lcd_block` for a dotted line:
  spi_open();
  LCD_TRUST_SET_ADDR((runmean >> (BIT_DISPARITY + LOG2_COMPRESS_GRAPH)), 0, (runmean >> (BIT_DISPARITY + LOG2_COMPRESS_GRAPH)), 127);
  for (uint16_t ij = 0; ij != 128; ++ij) {
    spi_send_16b((!dotted) - 1);
    INCR_DOTTED();
  }
  spi_close();
  INCR_DOTTED();

  uint8_t tgt;
  uint8_t pen = (hist[0] >> LOG2_COMPRESS_GRAPH);
  for (uint8_t i = 0; i != 127; ++i) {
    tgt = ((hist[i] = hist[i + 1]) >> LOG2_COMPRESS_GRAPH);
    if (pen == tgt) {
      lcd_look_to_the_cookie(pen, i, tgt, PULSELINE, BACKGROUND);
    } else if (pen > tgt) {
      lcd_look_to_the_cookie(tgt, i, pen - 1, PULSELINE, BACKGROUND);
    } else {
      lcd_look_to_the_cookie(pen + 1, i, tgt, PULSELINE, BACKGROUND);
    }
    pen = tgt;
  }

  hist[127] = (v >> BIT_DISPARITY);

  tgt = (hist[127] >> LOG2_COMPRESS_GRAPH);
  if (pen == tgt) {
    lcd_block(pen, 127, tgt, 127, PULSELINE);
  } else if (pen > tgt) {
    lcd_block(tgt, 127, pen - 1, 127, PULSELINE);
  } else {
    lcd_block(pen + 1, 127, tgt, 127, PULSELINE);
  }

  if (v > runmean) {
    lcd_block((runmean >> (BIT_DISPARITY + LOG2_COMPRESS_GRAPH)), 0, (runmean >> (BIT_DISPARITY + LOG2_COMPRESS_GRAPH)), 127, BACKGROUND);
    ++runmean;
  } else if (v < runmean) {
    falling = 0;
    GPIO_PULL(PIN_BUZZER, LO);
    lcd_block((runmean >> (BIT_DISPARITY + LOG2_COMPRESS_GRAPH)), 0, (runmean >> (BIT_DISPARITY + LOG2_COMPRESS_GRAPH)), 127, BACKGROUND);
    --runmean;
  }

  if ((v >> BIT_DISPARITY) > runpeak) {
    lcd_block(runpeak >> LOG2_COMPRESS_GRAPH, 0, runpeak >> LOG2_COMPRESS_GRAPH, 127, BACKGROUND); // Erase the last peak
    runpeak = (v >> BIT_DISPARITY);
    peakidx = 127;
  } else if (peakidx) {
    --peakidx;
  } else {
    // Iterate & find the new peak (unfortunately)
    lcd_block(runpeak >> LOG2_COMPRESS_GRAPH, 0, runpeak >> LOG2_COMPRESS_GRAPH, 127, BACKGROUND); // Erase the last peak
    runpeak = hist[0];
    peakidx = 0;
    for (uint8_t i = 1; i != 128; ++i) {
      if ((hist[i] << 3) >= runpeak) {
        runpeak = hist[i];
        peakidx = i;
      }
    }
  }
  lcd_block(runpeak >> LOG2_COMPRESS_GRAPH, 0, runpeak >> LOG2_COMPRESS_GRAPH, 127, BACKGROUND); // Erase the last peak

  if ((!falling) && ((v - runmean) > (((runpeak << BIT_DISPARITY) - runmean) >> 1U))) { // If we're more than halfway from mean to peak AND not already falling
    falling = 1;
    GPIO_PULL(PIN_BUZZER, HI);
    return 1;
  }
  return 0;
}

static void update_bpm(uint16_t /* just in a hell of a case (>255) */ bpm) {
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
