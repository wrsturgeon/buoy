#define NDEBUG 1

//%%%%%%%%%%%%%%%% CONFIGURABLE PARAMETERS:
#define CYCLES_PER_SECOND 16
#define LOG2_ADC_SAMPLES 4U // Set <= 4 so we use at most (2^4=)16 12-bit numbers and a 16-bit sum is juuuuust guaranteed not to overflow
#define LOG2_HEARTBEAT_SAMPLES 4U
//%%%%%%%%%%%%%%%% END CONFIGURABLE PARAMETERS

#define N_ADC_SAMPLES (1ULL << LOG2_ADC_SAMPLES)
#define N_HEARTBEAT_SAMPLES (1ULL << LOG2_HEARTBEAT_SAMPLES)
#define MASK_ADC_FULL (N_ADC_SAMPLES - 1U)

// Average resting heart rate ~60-100bpm
// We ought to be able to cover the vast, vast majority on either extreme
// For reasonable performance, say 256 cycles = 1 second
// So raw output >> 8 -> seconds
// The ESP32 timer runs at 80MHz before prescaling
// So with N samples in a cycle (over which we average out random fluctuations),
// we need to prescale by 80,000,000 / (CYCLES_PER_SECOND * N)
#define PRESCALE_BY (uint16_t)((80000000. / (((double)CYCLES_PER_SECOND) * N_ADC_SAMPLES)) + 0.5)

#include "graphics.h"
#include "hardware/adc.h"
#include "hardware/timing.h"
#include "hardware/watchdog.h"
#include "sane-assert.h"

static uint16_t ADC_SAMPLE_VECTOR[N_ADC_SAMPLES];
static uint16_t HEARTBEAT_SAMPLE_VECTOR[N_HEARTBEAT_SAMPLES];
static uint64_t NEXT_TICK = 0;
static uint64_t HEARTBEAT_INDEX; // Initial position doesn't matter since it's cyclic

__attribute__((always_inline)) uint16_t time_between_beats(void) {
  static uint16_t last; // Initial value doesn't matter (wrong whatever it is and gone in a few seconds)
  uint16_t const lastlast = last;
  last = timing_get_clock();
  return last - lastlast;
}

__attribute__((always_inline)) inline static uint16_t adc_mean(void) {
  SANE_ASSERT(!(NEXT_TICK & MASK_ADC_FULL)); // We shouldn't be computing the mean halfway through a cycle

  uint16_t sum = 0;

#pragma GCC unroll 64
  for (uint8_t i = 0; i != N_ADC_SAMPLES; ++i) { sum += ADC_SAMPLE_VECTOR[i]; }

  return (sum >> LOG2_ADC_SAMPLES);
}

__attribute__((always_inline)) inline static uint16_t heartbeat_mean(void) {
  uint16_t sum = 0;

#pragma GCC unroll 64
  for (uint8_t i = 0; i != N_HEARTBEAT_SAMPLES; ++i) { sum += HEARTBEAT_SAMPLE_VECTOR[i]; }

  return (sum >> LOG2_HEARTBEAT_SAMPLES);
}

__attribute__((always_inline)) inline static void once_every_tick(void) {
  ADC_SAMPLE_VECTOR[NEXT_TICK & MASK_ADC_FULL] = adc_poll();
}

__attribute__((always_inline)) inline static void once_every_cycle(void) {
  if (display_and_check_heartbeat(adc_mean())) {
    HEARTBEAT_SAMPLE_VECTOR[++HEARTBEAT_INDEX] = time_between_beats();
    // uint16_t mean;
    // {
    //   uint32_t sum = 0;
    //   for (uint16_t i = 0; i < (1U << LG_HBUF); ++i) { sum += cbuf_get_uint16_16(i, &heartbuf); }
    //   mean = (sum >> LG_HBUF);
    // }
    // // We want an operation s.t.
    // // (X) / (sec / 62,500) = beats / minute
    // // 62,500 X / sec = beats / (60 sec)
    // // 3,750,000 X = beats
    // // X = beats / 3,750,000
    // // and 3,750,000 >> 6 fits within 16b (58,593.75)
    // update_bpm(58594U / (mean >> 6));
  }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% FreeRTOS entry point instead of `main`, but using none of FreeRTOS's other features.
void app_main(void) {

  rtc_wdt_protect_off();
  rtc_wdt_disable();

  // Initialize modules
  graphics_init();
  adc_init();
  timing_init();

  // Main loop
  do {
    // SANE_ASSERT((timing_get_clock() - NEXT_TICK) < 16U); // subtraction into a signed integer so we don't always fail straddling overflow

    // TODO: timer interrupts (will also fix ADC sampling bias!)
    do { // twiddle our thumbs
      // printf("Waiting...\r\n");
    } while (timing_get_clock() < NEXT_TICK);

    once_every_tick();

    if (!(MASK_ADC_FULL & ++NEXT_TICK)) { once_every_cycle(); }

    // feed_watchdog();

  } while (1);
}
