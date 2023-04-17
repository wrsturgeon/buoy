#define NDEBUG 1

//%%%%%%%%%%%%%%%% CONFIGURABLE PARAMETERS:
#define LOG2_CYCLES_PER_SECOND 1U
#define LOG2_ADC_SAMPLES 0U // Set <= 4 so we use at most (2^4=)16 12-bit numbers and a 16-bit sum is juuuuust guaranteed not to overflow
#define LOG2_HEARTBEAT_SAMPLES 4U
//%%%%%%%%%%%%%%%% END CONFIGURABLE PARAMETERS

#define N_CYCLES_PER_SECOND (1ULL << LOG2_CYCLES_PER_SECOND)
#define N_ADC_SAMPLES (1ULL << LOG2_ADC_SAMPLES)
#define N_HEARTBEAT_SAMPLES (1ULL << LOG2_HEARTBEAT_SAMPLES)
#define MASK_ADC_FULL (N_ADC_SAMPLES - 1U)

// Average resting heart rate ~60-100bpm
// We ought to be able to cover the vast, vast majority on either extreme
// For reasonable performance, say 256 cycles = 1 second
// So raw output >> 8 -> seconds
// The ESP32 timer runs at 80MHz before prescaling
// So with N samples in a cycle (over which we average out random fluctuations),
// we need to prescale by 80,000,000 / (N_CYCLES_PER_SECOND * N)
#define PRESCALE_DENOMINATOR ((N_CYCLES_PER_SECOND) * (N_ADC_SAMPLES))
#define HALF_DENOMINATOR ((PRESCALE_DENOMINATOR + 1ULL) >> 1U)
#define PRESCALE_WHOLE_ENCHILADA ((80000000ULL + HALF_DENOMINATOR) / (PRESCALE_DENOMINATOR)) // Add half the denominator so we round up

#if PRESCALE_WHOLE_ENCHILADA <= 0
#error Prescaler must be positive
#endif

#if PRESCALE_WHOLE_ENCHILADA > (1ULL << (16U + 32U))
#error Prescaler must not be so ridiculously high as to impact performance
#endif

#include <stdint.h>

#if PRESCALE_WHOLE_ENCHILADA < (1ULL << 16U)
#define PRESCALE_OVERFLOW 0 // Boolean macro
#define PRESCALE_BY ((uint16_t)(PRESCALE_WHOLE_ENCHILADA))
#else // PRESCALE_WHOLE_ENCHILADA < (1ULL << 16U)
// 0 is a special case in hardware that divides by 65536
#define PRESCALE_BY 0
#define PRESCALE_OVERFLOW 1 // Boolean macro
#define PRESCALE_OVERFLOW_PERIOD ((PRESCALE_WHOLE_ENCHILADA + (1ULL << 15U)) >> 16U)
#if PRESCALE_WHOLE_ENCHILADA < (1ULL << 32U)
typedef uint16_t prescale_overflow_t;
#else  // PRESCALE_WHOLE_ENCHILADA < (1ULL << 32U)
typedef uint32_t prescale_overflow_t;
#endif // PRESCALE_WHOLE_ENCHILADA < (1ULL << 32U)
static prescale_overflow_t prescale_overflow = 0;
static prescale_overflow_t prescale_overflow_next = PRESCALE_OVERFLOW_PERIOD;
#endif // PRESCALE_WHOLE_ENCHILADA < (1ULL << 16U)

#include "graphics.h"
#include "hardware/adc.h"
#include "hardware/timing.h"
#include "sane-assert.h"

// Include just enough FreeRTOS to feed the watchdog timer
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static uint16_t adc_sample_vector[N_ADC_SAMPLES];
static uint16_t heartbeat_sample_vector[N_HEARTBEAT_SAMPLES];
static uint32_t this_tick = -1;
static uint8_t heartbeat_index; // Initial position doesn't matter since it's cyclic

__attribute__((always_inline)) inline static uint16_t time_between_beats(uint32_t const now) {
  static uint32_t last; // Initial value doesn't matter (wrong whatever it is and gone in a few seconds)
  uint32_t const lastlast = last;
  return (last = now) - lastlast;
}

__attribute__((always_inline)) inline static uint16_t adc_mean(void) {
  SANE_ASSERT(!(this_tick & MASK_ADC_FULL)); // We shouldn't be computing the mean halfway through a cycle

  uint16_t sum = 0;

#pragma GCC unroll 64
  for (uint8_t i = 0; i != N_ADC_SAMPLES; ++i) { sum += adc_sample_vector[i]; }

  return (sum >> LOG2_ADC_SAMPLES);
}

__attribute__((always_inline)) inline static uint16_t heartbeat_mean(void) {
  uint16_t sum = 0;

#pragma GCC unroll 64
  for (uint8_t i = 0; i != N_HEARTBEAT_SAMPLES; ++i) { sum += heartbeat_sample_vector[i]; }

  return (sum >> LOG2_HEARTBEAT_SAMPLES);
}

__attribute__((always_inline)) inline static void once_every_tick(void) {
  static uint8_t index;
  adc_sample_vector[MASK_ADC_FULL & ++index] = adc_poll();
}

__attribute__((always_inline)) inline static void once_every_cycle(void) {
  if (display_and_check_heartbeat(adc_mean())) {
    heartbeat_sample_vector[heartbeat_index] = time_between_beats(this_tick);
    if (N_HEARTBEAT_SAMPLES == ++heartbeat_index) { heartbeat_index = 0; }
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

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% FreeRTOS task instead of `main` (so we can feed the watchdog), but using no other FreeRTOS features.
void app_main(void) {

  // Initialize modules
  graphics_init();
  adc_init();
  timing_init();

  // Main loop
  do {

    // TODO: timer interrupts (will also fix ADC sampling bias!)
    while (timing_get_clock_32b() < ++this_tick) {
      // printf("Ahead of schedule\r\n");

      vTaskDelay(1); // Reset the watchdog so it doesn't fucking kill us every two seconds
      // (but let it kill us if we're very behind schedule and we never satisfy the condition above)

      do { // twiddle our thumbs
      } while (timing_get_clock_32b() < this_tick);
    }

    // SANE_ASSERT((now - this_tick) < 16U); // subtraction into a signed integer so we don't always fail straddling overflow

#if PRESCALE_OVERFLOW
    if (prescale_overflow_next != ++prescale_overflow) { continue; }
    prescale_overflow_next += PRESCALE_OVERFLOW_PERIOD;
#endif // PRESCALE_OVERFLOW

    once_every_tick();
    if (!(MASK_ADC_FULL & this_tick)) { once_every_cycle(); }

  } while (1);
}
