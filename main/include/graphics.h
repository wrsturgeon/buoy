#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "big-ascii.h"
#include "buffer.h"
#include "hardware/adc.h" // for ADC_BIT_WIDTH
#include "lcd.h"
#include "sane-assert.h"

#include <stdint.h>

#define VISIBLE_SETUP 0         // 0: no; 1: yes
#define PEAK_AND_MEDIAN_LINES 1 // ^

#if USE_BUZZER
#define PIN_BUZZER FEATHER_A1
#endif // USE_BUZZER

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
  big_B(HZMAX - PADDING, PADDING + 5 * (BIGASCII_W + 1), RED);
  big_P(HZMAX - PADDING, PADDING + 6 * (BIGASCII_W + 1), RED);
  big_M(HZMAX - PADDING, PADDING + 7 * (BIGASCII_W + 1), RED);

  GPIO_PULL(PIN_LIT, HI); // Let there be light!

#if USE_BUZZER
  GPIO_ENABLE_OUTPUT(PIN_BUZZER);
#endif // USE_BUZZER

#ifndef NDEBUG
  GRAPHICS_READY = 1;
#endif // NDEBUG
}

#define INCR_DOTTED()                          \
  do {                                         \
    if ((++dotted) == NDOTTED) { dotted = 0; } \
  } while (0)
__attribute__((always_inline)) inline static uint8_t display_and_check_heartbeat(uint8_t const vrsh) {
  static buffer_t hist;
  static uint8_t runmedian = 128;
  static uint8_t runpeak = 0;
  static uint8_t falling = 0; // if we recorded a heartbeat & are now waiting for this "bump" to finish
#if PEAK_AND_MEDIAN_LINES
  static uint8_t peakidx = 0;
  static uint8_t dotted = 0;
#endif // PEAK_AND_MEDIAN_LINES

  buffer_push(vrsh, &hist);

  lcd_block(0, 0, 127, 0, BACKGROUND); // Erase the left edge

#if PEAK_AND_MEDIAN_LINES
  // Adjust & draw the green peak line
  if (vrsh > runpeak) {
    lcd_block(runpeak >> LOG2_COMPRESS_GRAPH, 0, runpeak >> LOG2_COMPRESS_GRAPH, 127, BACKGROUND); // Erase the last peak
    runpeak = vrsh;
    peakidx = 127;
  } else if (peakidx) {
    --peakidx;
  } else {
    // Iterate & find the new peak
    lcd_block(runpeak >> LOG2_COMPRESS_GRAPH, 0, runpeak >> LOG2_COMPRESS_GRAPH, 127, BACKGROUND); // Erase the last peak
    runpeak = buffer_get(0, &hist);
    peakidx = 0;
    for (uint8_t i = 1; i != ST7735_H; ++i) {
      if (buffer_get(i, &hist) >= runpeak) {
        runpeak = buffer_get(i, &hist);
        peakidx = i;
      }
    }
  }
  lcd_block(runpeak >> LOG2_COMPRESS_GRAPH, 0, runpeak >> LOG2_COMPRESS_GRAPH, 127, PULSEPEAK); // Erase the last peak

  // Adjust & draw the dotted median line
  if (vrsh > runmedian) {
    if (runmedian & ((1ULL << LOG2_COMPRESS_GRAPH) - 1U)) { lcd_block((runmedian >> LOG2_COMPRESS_GRAPH), 0, (runmedian >> LOG2_COMPRESS_GRAPH), 127, BACKGROUND); }
    ++runmedian;
  } else if (vrsh < runmedian) {
    falling = 0;
#if USE_BUZZER
    GPIO_PULL(PIN_BUZZER, LO);
#endif // USE_BUZZER
    if ((~runmedian) & ((1ULL << LOG2_COMPRESS_GRAPH) - 1U)) { lcd_block((runmedian >> LOG2_COMPRESS_GRAPH), 0, (runmedian >> LOG2_COMPRESS_GRAPH), 127, BACKGROUND); }
    --runmedian;
  }
  spi_open();
  LCD_TRUST_SET_ADDR((runmedian >> LOG2_COMPRESS_GRAPH), 0, (runmedian >> LOG2_COMPRESS_GRAPH), 127);
  for (uint16_t ij = 0; ij != ST7735_H; ++ij) {
    spi_send_16b((!dotted) - 1);
    INCR_DOTTED();
  }
  spi_close();
  INCR_DOTTED();
#endif // PEAK_AND_MEDIAN_LINES

  { // Draw the main scrolling chart
    uint8_t tgt;
    uint8_t pen = (buffer_get(0, &hist) >> LOG2_COMPRESS_GRAPH);
    for (uint8_t i = 0; i != ST7735_H; ++i) {
      tgt = (buffer_get(i, &hist) >> LOG2_COMPRESS_GRAPH);
      if (pen == tgt) {
        lcd_look_to_the_cookie(pen, i, tgt, PULSELINE, BACKGROUND);
      } else if (pen > tgt) {
        lcd_look_to_the_cookie(tgt, i, pen - 1, PULSELINE, BACKGROUND);
      } else {
        lcd_look_to_the_cookie(pen + 1, i, tgt, PULSELINE, BACKGROUND);
      }
      pen = tgt;
    }

    // tgt = (buffer_get(0, &hist) >> LOG2_COMPRESS_GRAPH);
    // if (pen == tgt) {
    //   lcd_block(pen, 127, tgt, 127, PULSELINE);
    // } else if (pen > tgt) {
    //   lcd_block(tgt, 127, pen - 1, 127, PULSELINE);
    // } else {
    //   lcd_block(pen + 1, 127, tgt, 127, PULSELINE);
    // }
  }

  if ((!falling) && ((vrsh - runmedian) > ((runpeak - runmedian) >> 1U))) { // If we're more than halfway from median to peak AND not already falling
    falling = 1;
#if USE_BUZZER
    GPIO_PULL(PIN_BUZZER, HI);
#endif // USE_BUZZER
    return 1;
  }
  return 0;
}

__attribute__((always_inline)) inline static void update_bpm(uint16_t /* just in a hell of a case (>255) */ bpm) {
  lcd_block(HZMAX - PADDING - BIGASCII_H + 1, PADDING + BIGASCII_W, HZMAX - PADDING, PADDING + ((((4 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U) - 1, BACKGROUND);
  if (bpm < 1000) {
    if (bpm < 100) {
      if (bpm < 10) {
        big_integer(bpm, HZMAX - PADDING, PADDING + ((((3 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U), RED);
      } else {
        big_integer(bpm / 10U, HZMAX - PADDING, PADDING + ((((2 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U), RED);
        big_integer(bpm % 10U, HZMAX - PADDING, PADDING + ((((3 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U), RED);
      }
    } else {
      big_integer(bpm / 100U, HZMAX - PADDING, PADDING + ((((1 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U), RED);
      big_integer((bpm / 10U) % 10U, HZMAX - PADDING, PADDING + ((((2 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U), RED);
      big_integer(bpm % 10U, HZMAX - PADDING, PADDING + ((((3 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U), RED);
    }
  } else {
    big_question(HZMAX - PADDING, PADDING + ((((1 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U), RED);
    big_question(HZMAX - PADDING, PADDING + ((((2 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U), RED);
    big_question(HZMAX - PADDING, PADDING + ((((3 << 1U) | 1U) * (BIGASCII_W + 1)) >> 1U), RED);
  }
}

#endif // GRAPHICS_H
