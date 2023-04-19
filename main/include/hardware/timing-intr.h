#ifndef TIMING_H
#define TIMING_H

#include "hardware/reg.h"
#include "sane-assert.h"
#include "stringify.h"

#include <esp_attr.h>
#include <esp_intr_alloc.h>
#include <rom/ets_sys.h>
#include <soc/dport_reg.h>
#include <soc/timer_group_reg.h>
#include <xtensa/xtensa_api.h>

#define F_CPU 240000000 // 240MHz
#define F_APB 80000000  //  80MHz

#define TIMG_N 0
#define TIMER_N 0
#define PRESCALE_BY 0
#define ARBITRARY_VALUE 1 // for clarity of intention
#define TIMING_INTERRUPT_PERIOD ((uint64_t)256)
#define TIMING_ADC_PERIOD ((uint64_t)1)
_Static_assert(!(TIMING_INTERRUPT_PERIOD % TIMING_ADC_PERIOD));

#define INTR_TYPE LEVEL
#define INTR_N SANDWICH(ETS, PASTE(INTR_TYPE, _INTR_SOURCE)) // ETS_TG0_T1_INUM
#define INTR_CPU PRO
#define INTR_CPU_N PASTE(INTR_CPU, _CPU_NUM)

#define TIMG(...) TIMG_LITERAL(TIMER_N, __VA_ARGS__)
#define TIMG_LITERAL(N, ...) TIMG_LITERAL_LITERAL(N, __VA_ARGS__)
#define TIMG_LITERAL_LITERAL(N, ...) TIMG_T##N##_##__VA_ARGS__ // ESP32 timer registers are formatted like `TIMG_T0_...` for timer #0

#define TIMG_REG(...) TIMG_REG_LITERAL(TIMER_N, TIMG_N, __VA_ARGS__)
#define TIMG_REG_LITERAL(N, GN, ...) TIMG_REG_LITERAL_LITERAL(N, GN, __VA_ARGS__)
#define TIMG_REG_LITERAL_LITERAL(N, GN, ...) REG(TIMG_T##N##__VA_ARGS__##_REG(GN))

#define SANDWICH(BEFORE, AFTER) SANDWICH_LITERAL(BEFORE, TIMG_N, TIMER_N, AFTER)
#define SANDWICH_LITERAL(BEFORE, GN, N, AFTER) SANDWICH_LITERAL_LITERAL(BEFORE, GN, N, AFTER)
#define SANDWICH_LITERAL_LITERAL(BEFORE, GN, N, AFTER) BEFORE##_TG##GN##_T##N##_##AFTER

#define SANDWICH_CPU(BEFORE, AFTER) SANDWICH_CPU_LITERAL(BEFORE, INTR_CPU, AFTER)
#define SANDWICH_CPU_LITERAL(BEFORE, NCPU, AFTER) SANDWICH_CPU_LITERAL_LITERAL(BEFORE, NCPU, AFTER)
#define SANDWICH_CPU_LITERAL_LITERAL(BEFORE, NCPU, AFTER) SANDWICH(BEFORE##_##NCPU, AFTER)

#define SANDWICH_DPORT(AFTER) SANDWICH_DPORT_LITERAL(INTR_CPU, TIMG_N, TIMER_N, INTR_TYPE, AFTER)
#define SANDWICH_DPORT_LITERAL(NCPU, GN, N, ITYPE, AFTER) SANDWICH_DPORT_LITERAL_LITERAL(NCPU, GN, N, ITYPE, AFTER)
#if TIMG_N
#define SANDWICH_DPORT_LITERAL_LITERAL(NCPU, GN, N, ITYPE, AFTER) DPORT_##NCPU##_TG##GN##_T##N##_##ITYPE##_##AFTER
#else // TIMG_N
#define SANDWICH_DPORT_LITERAL_LITERAL(NCPU, GN, N, ITYPE, AFTER) DPORT_##NCPU##_TG_T##N##_##ITYPE##_##AFTER
#endif // TIMG_N

#ifndef NDEBUG
static uint8_t TIMING_READY = 0;
// DRAM_ATTR: accessible from interrupts
static DRAM_ATTR uint8_t TIMING_INTERRUPTS_RECEIVED = 0;
#endif // NDEBUG

__attribute__((always_inline)) inline static void timing_set_clock(uint64_t value) {
  SANE_ASSERT(!TIMING_READY); // Don't use this after setup! It'd fuck with the interrupt timing
  TIMG_REG(LOADHI) = (value >> 32U);
  TIMG_REG(LOADLO) = value;
  TIMG_REG(LOAD) = ARBITRARY_VALUE;
}

__attribute__((always_inline)) inline static uint64_t timing_get_clock(void) {
  SANE_ASSERT(TIMING_READY);
  TIMG_REG(UPDATE) = ARBITRARY_VALUE;
  return ((((uint64_t)TIMG_REG(HI)) << 32U) | TIMG_REG(LO));
}

__attribute__((always_inline)) inline static void timing_enable_alarm(void) {
  TIMG_REG(CONFIG) |= TIMG(ALARM_EN_M); // Re-enable since it's automatically cleared every time (Manual p. 500, $18.2.3)
}

__attribute__((always_inline)) inline static void timing_disable_alarm(void) {
  TIMG_REG(CONFIG) &= ~TIMG(ALARM_EN_M);
}

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/memory-types.html#iram-instruction-ram
void IRAM_ATTR clock_interrupt(void*) {
  ++TIMING_INTERRUPTS_RECEIVED;
  ets_printf("ISR: #=%u\r\n", TIMING_INTERRUPTS_RECEIVED);
  SANE_ASSERT(TIMING_INTERRUPTS_RECEIVED); // overflow bad (in context)
}

static void timing_respond_to_interrupt(void) {
  static DRAM_ATTR uint64_t NEXT_INTERRUPT = 0;
  NEXT_INTERRUPT += TIMING_INTERRUPT_PERIOD; // overflow doesn't matter on purpose
  TIMG_REG(ALARMHI) = (NEXT_INTERRUPT >> 32U);
  TIMG_REG(ALARMLO) = NEXT_INTERRUPT;
  ets_printf("Clock interrupt! Next @ %llu\r\n", NEXT_INTERRUPT);
  --TIMING_INTERRUPTS_RECEIVED;
  timing_enable_alarm();
}

__attribute__((always_inline)) inline static void timing_init(void) {
  SANE_ASSERT(!TIMING_READY);

  // Disable the clock to work on it
  // p. 499, $18.2.1
  TIMG_REG(CONFIG) &= ~TIMG(EN_M);

  // REG(SANDWICH_DPORT(INT_MAP_REG)) &= ~SANDWICH_DPORT(INT_MAP_M);
  // REG(SANDWICH_DPORT(INT_MAP_REG)) |= (SANDWICH(ETS, PASTE(INTR_TYPE, _INTR_SOURCE)) << SANDWICH_DPORT(INT_MAP_S));

  // intr_matrix_set(INTR_CPU_N, SANDWICH(ETS, PASTE(INTR_TYPE, _INTR_SOURCE)), INTR_N);
  // xt_set_interrupt_handler(INTR_N, clock_interrupt, 0);
  // xt_ints_on((1ULL << INTR_N));

  TIMG_REG(CONFIG) &= ~TIMG(DIVIDER_M); // Clear prescaler so we can |= it
  TIMG_REG(CONFIG) |= (TIMG(INCREASE_M) | TIMG(AUTORELOAD_M) | TIMG(PASTE(INTR_TYPE, _INT_EN_M)) | (PRESCALE_BY << TIMG(DIVIDER_S)));
  timing_set_clock(0);

  // TIMG_REG(ALARMHI) = (TIMING_INTERRUPT_PERIOD >> 32U);
  // TIMG_REG(ALARMLO) = (uint32_t)TIMING_INTERRUPT_PERIOD;
  TIMG_REG(ALARMHI) = 0;
  TIMG_REG(ALARMLO) = 0;

#ifndef NDEBUG
  TIMING_READY = 1;
#endif // NDEBUG

  esp_intr_alloc(SANDWICH(ETS, PASTE(INTR_TYPE, _INTR_SOURCE)), ESP_INTR_FLAG_IRAM, clock_interrupt, 0, 0);
  timing_enable_alarm();

  // Re-enable the clock
  TIMG_REG(CONFIG) |= TIMG(EN_M);
}

#endif // TIMING_H
