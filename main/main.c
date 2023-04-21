//%%%%%%%%%%%%%%%% CONFIGURABLE PARAMETERS:
// #define NDEBUG 1                  // Boolean macro
#define USE_BUZZER 1              // Boolean macro
#define SCREEN_FLIPPED_OVER_MCU 0 // Whether the screen is nicely aligned next to the ESP32 (=0) or flipped to lay on top of it (=1)
#define LOG2_CYCLES_PER_SECOND 5U // e.g. `5U` -> (2^5=)32 cycles per second
#define LOG2_ADC_SAMPLES 0U       // Set <= 4 so we use at most (2^4=)16 12-bit numbers and a 16-bit sum is juuuuust guaranteed not to overflow
#define LOG2_HEARTBEAT_SAMPLES 4U // Also <= 4: how many heartbeats to remember in BPM calculation
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

#define BPM_NUMERATOR (60ULL << (LOG2_CYCLES_PER_SECOND + LOG2_HEARTBEAT_SAMPLES))
#if BPM_NUMERATOR >= (1ULL << 16U)
#error BPM_NUMERATOR has to fit into a 16-bit unsigned integer
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

#include <sdkconfig.h>

#include "graphics.h"
#include "hardware/adc.h"
#include "hardware/timing.h"
#include "not-bare-metal/tasks.h"
#include "sane-assert.h"

static uint16_t adc_sample_vector[N_ADC_SAMPLES];
static uint16_t heartbeat_sample_vector[N_HEARTBEAT_SAMPLES];
static uint32_t this_tick = -1;
static uint32_t this_cycle = -1;
static uint8_t heartbeat_index; // Initial position doesn't matter since it's cyclic

__attribute__((always_inline)) inline static uint16_t cycles_between_beats(void) {
  static uint32_t last = 0; // Initial value doesn't matter (wrong whatever it is and gone in a few seconds)
  uint32_t const lastlast = last;
  return (last = this_cycle) - lastlast;
}

__attribute__((always_inline)) inline static uint16_t adc_mean(void) {
  SANE_ASSERT(!(this_tick & MASK_ADC_FULL)); // We shouldn't be computing the mean halfway through a cycle

  uint16_t sum = 0;

#pragma GCC unroll 16
  for (uint8_t i = 0; i != N_ADC_SAMPLES; ++i) { sum += adc_sample_vector[i]; }

  return (sum >> LOG2_ADC_SAMPLES);
}

__attribute__((always_inline)) inline static void once_every_tick(void) {
  adc_sample_vector[MASK_ADC_FULL & this_tick] = adc_poll();
}

_Static_assert(ADC_BIT_WIDTH > 8);
__attribute__((always_inline)) inline static void once_every_cycle(void) {
  if (display_and_check_heartbeat(adc_mean() >> (ADC_BIT_WIDTH - 8U))) {
    heartbeat_sample_vector[heartbeat_index] = cycles_between_beats();
    if (N_HEARTBEAT_SAMPLES == ++heartbeat_index) { heartbeat_index = 0; }

    uint16_t sum = 0;
#pragma GCC unroll 16
    for (uint8_t i = 0; i != N_HEARTBEAT_SAMPLES; ++i) { sum += heartbeat_sample_vector[i]; }

    if (sum) { update_bpm((BPM_NUMERATOR + (sum >> 1U)) / sum); } // BPM_NUMERATOR is <<'d by LOG2_HEARTBEAT_SAMPLES instead of dividing the denominator to make a sum into a mean
  }
}

void non_wifi_tasks(void* /* unused */) {

  // Initialize modules
  graphics_init();
  adc_init();
  timing_init();

  // Main loop
  do {

    // TODO(wrsturgeon): timer interrupts (will also fix ADC sampling bias!)
    while (timing_get_clock_32b() < ++this_tick) {
      // Ahead of schedule!

      vTaskDelay(1); // Reset the watchdog so it doesn't fucking kill us every two seconds
      // (but let it kill us if we're very behind schedule and we never satisfy the condition above)

      do { // twiddle our thumbs
      } while (timing_get_clock_32b() < this_tick);
    }

#if PRESCALE_OVERFLOW
    if (prescale_overflow_next != ++prescale_overflow) { continue; }
    prescale_overflow_next += PRESCALE_OVERFLOW_PERIOD;
#endif // PRESCALE_OVERFLOW

    once_every_tick();
    if (!(MASK_ADC_FULL & this_tick)) {
      ++this_cycle;
      once_every_cycle();
    }

  } while (1);
}

void app_main(void) {

  start_freertos_tasks(non_wifi_tasks);

  // Halt, but don't return
  do {
    vTaskDelay(-1);
  } while (1);
}
