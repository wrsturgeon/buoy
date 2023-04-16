#ifndef ADC_H
#define ADC_H

// Manual p. 627
// https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf

#include "hardware/reg.h"
#include "stringify.h"

#include <soc/adc_channel.h>
#include <soc/rtc_io_channel.h>
#include <soc/rtc_io_reg.h>
#include <soc/sens_reg.h>
#include <sys/lock.h>

#define ADC_USING_FREERTOS 0
#if ADC_USING_FREERTOS
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#endif

// Manual p. 639, not sure why they're this way
#define ATTEN_0DB 0b00
#define ATTEN_1DB 0b11
#define ATTEN_3DB 0b01
#define ATTEN_6DB 0b10

// Configurable!
#define ADC_UNIT_1IDX 1 // One-indexed, so ADC1 -> 1, ADC2 -> 2
_Static_assert(ADC_UNIT_1IDX);
#define ADC_UNIT_0IDX (ADC_UNIT_1IDX - 1) // Minus one! So ADC1 -> 0, ADC2 -> 1
#define ADC_CHANNEL 0
#define ADC_ATTEN 0
#define SAR_PRESCALE 2

#if ADC_UNIT_1IDX == 1
#define ADC_CTRLREG REG(SENS_SAR_READ_CTRL_REG)
#elif ADC_UNIT_1IDX == 2
#define ADC_CTRLREG REG(SENS_SAR_READ_CTRL2_REG)
#else
#error "Unrecognized ADC unit (should be 1 or 2)"
#endif

#define ADC_PINN PASTE(PASTE(PASTE(PASTE(ADC, ADC_UNIT_1IDX), _CHANNEL_), ADC_CHANNEL), _GPIO_NUM) // e.g. ADC1_CHANNEL_0_GPIO_NUM, from <soc/adc_channel.h>
#define RTC_CHANNEL PASTE(PASTE(RTCIO_GPIO, ADC_PINN), _CHANNEL)

static uint8_t ADC_READY = 0;

#if ADC_USING_FREERTOS
extern portMUX_TYPE rtc_spinlock; // Built in to the ESP32 link process, unfortunately
#endif

__attribute__((always_inline)) inline static void adc_init(void) {
  assert(!ADC_READY);
  // Most registers on p. 638 of the manual, but every line with a register below is documented inline

#if ADC_USING_FREERTOS
  vPortEnterCritical(&rtc_spinlock); // See note on `rtc_spinlock` declaration above
#endif

  REG(SENS_SAR_START_FORCE_REG) |= PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _BIT_WIDTH_M);                        // Both bits high: 12b signal
  ADC_CTRLREG |= PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _SAMPLE_BIT_M);                                         // Ditto ^^^^^^^^: 12b signal
  REG(RTC_IO_SENSOR_PADS_REG) |= RTC_IO_SENSE1_MUX_SEL_M;                                                      // p. 82: Route SENSE1 to RTC rather than DIG(ital)
  REG(RTC_IO_SENSOR_PADS_REG) &= ~RTC_IO_SENSE1_FUN_SEL_M;                                                     // p. 82: RTC IO_MUX selects function #0
  REG(PASTE(PASTE(RTC_GPIO_PIN, RTC_CHANNEL), _REG)) &= ~PASTE(PASTE(RTC_GPIO_PIN, RTC_CHANNEL), _PAD_DRIVER); // p. 80: Normal output, not open-drain
  REG(RTC_GPIO_ENABLE_REG) &= ~(1U << (RTC_CHANNEL + RTC_GPIO_ENABLE_S));                                      // p. 78: Disable GPIO function for our ADC input
  REG(RTC_IO_SENSOR_PADS_REG) &= ~RTC_IO_SENSE1_FUN_IE_M;
#if ADC_UNIT_1IDX == 1
  // Now this is just completely undocumented--even the memory address is ostensibly nonexistent in the manual
  REG(SENS_SAR_MEAS_CTRL2_REG) &= ~PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _DAC_XPD_FSM_M); // From `dac_ll_rtc_sync_by_adc(false)`
  REG(RTC_IO_HALL_SENS_REG) &= ~RTC_IO_XPD_HALL_M;                                        // Disable the Hall sensor, which might want to steal ADC1 from us
  REG(SENS_SAR_MEAS_WAIT2_REG) &= ~SENS_FORCE_XPD_AMP_M;                                  // Clear AMP settings to |= them below
  REG(SENS_SAR_MEAS_WAIT2_REG) |= (SENS_FORCE_XPD_AMP_PD << SENS_FORCE_XPD_AMP_S);        // Disable AMP, which also might apparently steal
  REG(SENS_SAR_MEAS_CTRL_REG) &= ~(SENS_AMP_RST_FB_FSM_S | SENS_AMP_SHORT_REF_FSM_M | SENS_AMP_SHORT_REF_GND_FSM_M);
  REG(SENS_SAR_MEAS_WAIT1_REG) |= (SENS_SAR_AMP_WAIT1_M | SENS_SAR_AMP_WAIT2_M | SENS_SAR_AMP_WAIT3_M);
#elif ADC_UNIT_1IDX == 2
  REG(SENS_SAR_START_FORCE_REG) &= ~SENS_SAR2_PWDET_CCT_M;        // Power detector capacitance tuning--
  REG(SENS_SAR_START_FORCE_REG) |= (4U << SENS_SAR2_PWDET_CCT_S); //   ...I'm trusting the manual on this one
#else
#error "Unrecognized ADC unit (should be 1 or 2)"
#endif
  ADC_CTRLREG |= PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _DATA_INV_M);                  // Invert ADC data
  ADC_CTRLREG &= ~PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _CLK_DIV_M);                  // Clear prescaler so we can |= it
  ADC_CTRLREG |= (SAR_PRESCALE << PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _CLK_DIV_S)); // |= it

  REG(PASTE(PASTE(SENS_SAR_ATTEN, ADC_UNIT_1IDX), _REG)) &= ~(0x3 << (ADC_CHANNEL << 1U));              // Clear attenuation so we can |= it
  REG(PASTE(PASTE(SENS_SAR_ATTEN, ADC_UNIT_1IDX), _REG)) |= ((ADC_ATTEN & 0x3) << (ADC_CHANNEL << 1U)); // |=

#if ADC_USING_FREERTOS
  vPortExitCritical(&rtc_spinlock); // See note on `rtc_spinlock` declaration above
#endif

  ADC_READY = 1;
}

__attribute__((always_inline)) inline static int16_t adc_poll(void) {
  assert(ADC_READY);

#if ADC_USING_FREERTOS
  vPortEnterCritical(&rtc_spinlock);
#endif

  // Force SAR power on, regardless of FSM
  REG(SENS_SAR_MEAS_WAIT2_REG) |= SENS_FORCE_XPD_SAR_M; // This register is also entirely absent in the manual, but see ~/esp/esp-idf/components/hal/esp32/include/hal/sar_ctrl_ll.h:42 (`sar_ctrl_ll_set_power_mode`)

  // Flip some settings (TODO: I have a sneaking suspicion that this is duplicated setup)
  ADC_CTRLREG &= ~PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _DIG_FORCE_M);                                                                     // Don't force digital (we're using RTC)
  REG(SENS_SAR_MEAS_START1_REG) |= (SENS_MEAS1_START_FORCE_M | PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _EN_PAD_FORCE_M));                    // Control with software instead of ULP
  REG(SENS_SAR_TOUCH_CTRL1_REG) |= (SENS_XPD_HALL_FORCE_M | SENS_HALL_PHASE_FORCE_M);                                                      // Software control over the Hall sensor as well
  REG(PASTE(PASTE(SENS_SAR_MEAS_START, ADC_UNIT_1IDX), _REG)) &= ~PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _EN_PAD_M);                        // Clear all channels so we can select one below
  REG(PASTE(PASTE(SENS_SAR_MEAS_START, ADC_UNIT_1IDX), _REG)) |= (1U << (ADC_CHANNEL + PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _EN_PAD_S))); // Clear all channels so we can select one below

  REG(PASTE(PASTE(SENS_SAR_MEAS_START, ADC_UNIT_1IDX), _REG)) &= ~PASTE(PASTE(SENS_SAR, ADC_UNIT_1IDX), _EN_PAD_M);
  REG(PASTE(PASTE(SENS_SAR_MEAS_START, ADC_UNIT_1IDX), _REG)) |= (1ULL << ADC_CHANNEL);

#if ADC_UNIT_1IDX == 1
  do {
  } while (REG(SENS_SAR_SLAVE_ADDR1_REG) & SENS_MEAS_STATUS_M); // This bitfield is supposed to be reserved (p. 668)!!! what the fuck?
#endif
  _Static_assert(SENS_MEAS1_START_SAR_M == SENS_MEAS2_START_SAR_M);                       // We arbitrarily use MEAS1 below
  REG(PASTE(PASTE(SENS_SAR_MEAS_START, ADC_UNIT_1IDX), _REG)) &= ~SENS_MEAS1_START_SAR_M; // Finish (the last) measurement (to be safe)
  REG(PASTE(PASTE(SENS_SAR_MEAS_START, ADC_UNIT_1IDX), _REG)) |= SENS_MEAS1_START_SAR_M;  // Begin measurement

  _Static_assert(SENS_MEAS1_DONE_SAR_M == SENS_MEAS2_DONE_SAR_M);
  do { // wait for the oven to ding
  } while (REG(PASTE(PASTE(SENS_SAR_MEAS_START, ADC_UNIT_1IDX), _REG)) & SENS_MEAS1_DONE_SAR_M);

  _Static_assert(SENS_MEAS1_DATA_SAR_M == SENS_MEAS2_DATA_SAR_M);
  int16_t adc_value = ((REG(PASTE(PASTE(SENS_SAR_MEAS_START, ADC_UNIT_1IDX), _REG)) >> SENS_MEAS1_DATA_SAR_S) & SENS_MEAS1_DATA_SAR_V);

  // Cede SAR power control back to FSM
  REG(SENS_SAR_MEAS_WAIT2_REG) &= ~SENS_FORCE_XPD_SAR_M; // Mirrors the first line of the critical section: also completely undocumented

#if ADC_USING_FREERTOS
  vPortExitCritical(&rtc_spinlock);
#endif

  return adc_value;
}

#endif // ADC_H
