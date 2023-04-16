#ifndef TIMING_H
#define TIMING_H

#include "bitbang/reg.h"

#include <soc/timer_group_reg.h>

#define F_CPU 240000000 // 240MHz
#define F_APB 80000000  //  80MHz

#define TIMG_N 0
#define TIMER_N 0
#define PRESCALE_BY 0
#define ARBITRARY_VALUE 1 // for clarity of intention

#define TIMG(...) TIMG_LITERAL(TIMER_N, __VA_ARGS__)
#define TIMG_LITERAL(N, ...) TIMG_LITERAL_LITERAL(N, __VA_ARGS__)
#define TIMG_LITERAL_LITERAL(N, ...) TIMG_T##N##_##__VA_ARGS__ // ESP32 timer registers are formatted like `TIMG_T0_...` for timer #0

#define TIMG_REG(...) TIMG_REG_LITERAL(TIMER_N, TIMG_N, __VA_ARGS__)
#define TIMG_REG_LITERAL(N, GN, ...) TIMG_REG_LITERAL_LITERAL(N, GN, __VA_ARGS__)
#define TIMG_REG_LITERAL_LITERAL(N, GN, ...) REG(TIMG_T##N##__VA_ARGS__##_REG(GN))

#ifndef NDEBUG
static uint8_t TIMING_READY = 0;
#endif // NDEBUG

__attribute__((always_inline)) inline static void timing_set_clock(uint64_t value) {
  assert(!TIMING_READY); // Don't use this after setup! It'd fuck with the interrupt timing
  TIMG_REG(LOADHI) = (value >> 32U);
  TIMG_REG(LOADLO) = value;
  TIMG_REG(LOAD) = ARBITRARY_VALUE;
}

__attribute__((always_inline)) inline static uint64_t timing_get_clock(void) {
  assert(TIMING_READY);
  TIMG_REG(UPDATE) = ARBITRARY_VALUE;
  return ((((uint64_t)TIMG_REG(HI)) << 32U) | TIMG_REG(LO));
}

__attribute__((always_inline)) inline static void timing_init(void) {
  assert(!TIMING_READY);

  // Disable the clock to work on it
  // p. 499, $18.2.1
  TIMG_REG(CONFIG) &= ~TIMG(EN_M);

  TIMG_REG(CONFIG) &= ~(TIMG(AUTORELOAD_M) | TIMG(DIVIDER_M) | TIMG(EDGE_INT_EN_M) | TIMG(LEVEL_INT_EN_M) | TIMG(ALARM_EN_M));
  TIMG_REG(CONFIG) |= (TIMG(INCREASE_M) | (PRESCALE_BY << TIMG(DIVIDER_S)));
  timing_set_clock(0);

  // Re-enable the clock
  TIMG_REG(CONFIG) |= TIMG(EN_M);

#ifndef NDEBUG
  TIMING_READY = 1;
#endif // NDEBUG
}

#endif // TIMING_H
