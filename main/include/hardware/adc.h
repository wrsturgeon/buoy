#ifndef ADCDUINO_H
#define ADCDUINO_H

#include "hardware/reg.h"
#include "sane-assert.h"
#include "stringify.h"

#include <soc/adc_channel.h>
#include <soc/gpio_reg.h>
#include <soc/io_mux_reg.h>
#include <soc/rtc_io_periph.h>
#include <soc/sens_reg.h>
#include <soc/syscon_reg.h>

#define ADC_ATTENUATION 0 // 3
_Static_assert((ADC_ATTENUATION & 3U) == ADC_ATTENUATION, "ADC attenuation must be on [0..3]");
#define ADC_CLK_DIV 2
#define ADC_BIT_WIDTH 12
#define ADC_PIN FEATHER_A4
#define ADC_INVERT 0

#define ADC_CHANNEL PASTE(PASTE(ADC1_GPIO, ADC_PIN), _CHANNEL)
#define ADC_IO_REG PASTE(PASTE(IO_MUX_GPIO, ADC_PIN), _REG)
#define RTC_IO_CHANNEL PASTE(PASTE(RTCIO_GPIO, ADC_PIN), _CHANNEL)

#ifndef NDEBUG
static uint8_t ADC_READY = 0;
#endif // NDEBUG

extern rtc_io_desc_t const rtc_io_desc[SOC_RTCIO_PIN_COUNT];

// not sure why, but some registers require 32-bit atomic access
__attribute__((always_inline)) inline static uint32_t force_32b_read(uint32_t const volatile* const restrict reg, uint32_t mask) {
  uint32_t volatile v32b = *reg;
  return v32b & mask;
}

__attribute__((always_inline)) inline static void force_32b_set(uint32_t volatile* const restrict reg, uint32_t mask) {
  uint32_t volatile v32b = *reg;
  *reg = (v32b |= mask);
}

__attribute__((always_inline)) inline static void force_32b_clear(uint32_t volatile* const restrict reg, uint32_t mask) {
  uint32_t volatile v32b = *reg;
  *reg = (v32b &= ~mask);
}

__attribute__((always_inline)) inline static void force_32b_clear_and_set(uint32_t volatile* const restrict reg, uint32_t mask, uint32_t set_mask) {
  uint32_t volatile v32b = *reg;
  v32b &= ~mask;
  *reg = (v32b |= set_mask);
}

__attribute__((always_inline)) inline static void adc_set_bit_width(void) {
  REG(SENS_SAR_START_FORCE_REG) |= SENS_SAR1_BIT_WIDTH_M; // 0x3 => full 12 bits
  REG(SENS_SAR_READ_CTRL_REG) |= SENS_SAR1_SAMPLE_BIT_M;  // ditto ^
}

__attribute__((always_inline)) inline static void adc_prescale(void) {
  force_32b_clear_and_set(REG_PTR(SYSCON_SARADC_CTRL_REG), SYSCON_SARADC_SAR_CLK_DIV_M, (ADC_CLK_DIV << SYSCON_SARADC_SAR_CLK_DIV_S));
}

__attribute__((always_inline)) inline static void adc_rtc_disable_gpio(void) {
  REG(rtc_io_desc[RTC_IO_CHANNEL].reg) &= ~rtc_io_desc[RTC_IO_CHANNEL].mux;
}

__attribute__((always_inline)) inline static void adc_disable_gpio_interrupt(void) {
  REG(PASTE(PASTE(GPIO_PIN, ADC_PIN), _REG)) &= ~(PASTE(PASTE(GPIO_PIN, ADC_PIN), _INT_TYPE_M) | PASTE(PASTE(GPIO_PIN, ADC_PIN), _INT_ENA_M));
#if (ADC_PIN < 32)
  REG(GPIO_STATUS_W1TC_REG) = (1ULL << ADC_PIN);
#else
  REG(GPIO_STATUS1_W1TC_REG) = (1ULL << (ADC_PIN - 32U));
#endif
}

__attribute__((always_inline)) inline static void adc_set_iomux_fn(void) {
  REG(ADC_IO_REG) &= ~MCU_SEL_M;
  REG(ADC_IO_REG) |= (PIN_FUNC_GPIO << MCU_SEL_S);
}

__attribute__((always_inline)) inline static void adc_redirect_from_gpio(void) {
  adc_rtc_disable_gpio();
  adc_disable_gpio_interrupt();
  adc_set_iomux_fn();
}

__attribute__((always_inline)) inline static void adc_rtc_gpio_set_direction(void) {
  REG(PASTE(PASTE(RTC_GPIO_PIN, RTC_IO_CHANNEL), _REG)) &= ~PASTE(PASTE(RTC_GPIO_PIN, RTC_IO_CHANNEL), _PAD_DRIVER_M);
  REG(RTC_GPIO_ENABLE_REG) &= ~(1U << RTC_IO_CHANNEL);
  REG(rtc_io_desc[RTC_IO_CHANNEL].reg) &= ~rtc_io_desc[RTC_IO_CHANNEL].ie;
}

__attribute__((always_inline)) inline static void adc_rtc_gpio_disable_pulldown(void) { REG(rtc_io_desc[RTC_IO_CHANNEL].reg) &= ~rtc_io_desc[RTC_IO_CHANNEL].pulldown; }
__attribute__((always_inline)) inline static void adc_rtc_gpio_disable_pullup(void) { REG(rtc_io_desc[RTC_IO_CHANNEL].reg) &= ~rtc_io_desc[RTC_IO_CHANNEL].pullup; }

__attribute__((always_inline)) inline static void adc_rtc_init_gpio(void) {
  REG(rtc_io_desc[RTC_IO_CHANNEL].reg) |= rtc_io_desc[RTC_IO_CHANNEL].mux;
  REG(rtc_io_desc[RTC_IO_CHANNEL].reg) &= ~(RTC_IO_TOUCH_PAD1_FUN_SEL_V << rtc_io_desc[RTC_IO_CHANNEL].func);
  adc_rtc_gpio_set_direction();
  adc_rtc_gpio_disable_pulldown();
  adc_rtc_gpio_disable_pullup();
}

__attribute__((always_inline)) inline static void adc_disable_hall_sensor(void) { REG(RTC_IO_HALL_SENS_REG) &= ~RTC_IO_XPD_HALL_M; }

__attribute__((always_inline)) inline static void adc_disable_amp(void) {
  REG(SENS_SAR_MEAS_WAIT2_REG) &= ~SENS_FORCE_XPD_AMP_M;
  REG(SENS_SAR_MEAS_WAIT2_REG) |= (SENS_FORCE_XPD_AMP_PD << SENS_FORCE_XPD_AMP_S); // PD: power down
  REG(SENS_SAR_MEAS_CTRL_REG) &= ~(SENS_AMP_RST_FB_FSM_M | SENS_AMP_SHORT_REF_FSM_M | SENS_AMP_SHORT_REF_GND_FSM_M);
  force_32b_clear_and_set(REG_PTR(SENS_SAR_MEAS_WAIT1_REG), SENS_SAR_AMP_WAIT1_M, 1ULL << SENS_SAR_AMP_WAIT1_S);
  force_32b_clear_and_set(REG_PTR(SENS_SAR_MEAS_WAIT1_REG), SENS_SAR_AMP_WAIT2_M, 1ULL << SENS_SAR_AMP_WAIT2_S);
  force_32b_clear_and_set(REG_PTR(SENS_SAR_MEAS_WAIT2_REG), SENS_SAR_AMP_WAIT3_M, 1ULL << SENS_SAR_AMP_WAIT3_S);
}

__attribute__((always_inline)) inline static void adc_set_attenuation(void) {
  adc_rtc_init_gpio();

  REG(SENS_SAR_MEAS_CTRL2_REG) &= ~SENS_SAR1_DAC_XPD_FSM_M; // Decouple DAC from ADC
  REG(SENS_SAR_READ_CTRL_REG)
#if ADC_INVERT // Inversions themselves are flipped: for whatever reason, this ADC channel needs to be inverted to be upright
      &= ~
#else  // ADC_INVERT
      |=
#endif // ADC_INVERT
         SENS_SAR1_DATA_INV_M;
  force_32b_clear_and_set(REG_PTR(SENS_SAR_READ_CTRL_REG), SENS_SAR1_CLK_DIV_M, (ADC_CLK_DIV << SENS_SAR1_CLK_DIV_S));
  adc_disable_hall_sensor();
  // adc_disable_amp();

  // adc_oneshot_ll_set_atten(ADC_UNIT_1, ADC_CHANNEL, ADC_ATTENUATION);
  REG(SENS_SAR_ATTEN1_REG) &= ~(3U << (ADC_CHANNEL << 1U));
  REG(SENS_SAR_ATTEN1_REG) |= (ADC_ATTENUATION << (ADC_CHANNEL << 1U));
}

__attribute__((always_inline)) inline static void adc_set_software_control(void) {
  REG(SENS_SAR_READ_CTRL_REG) &= ~SENS_SAR1_DIG_FORCE_M;                                  // RTC control instead of digital
  REG(SENS_SAR_MEAS_START1_REG) |= (SENS_MEAS1_START_FORCE_M | SENS_SAR1_EN_PAD_FORCE_M); // Software control instead of ULP control
  REG(SENS_SAR_TOUCH_CTRL1_REG) |= (SENS_XPD_HALL_FORCE_M | SENS_HALL_PHASE_FORCE_M);     // Software control of Hall sensor as well (so we can shut it up)
}

__attribute__((always_inline)) inline static void adc_set_channel(void) {
  REG(SENS_SAR_MEAS_START1_REG) &= ~SENS_SAR1_EN_PAD_M;
  REG(SENS_SAR_MEAS_START1_REG) |= (1ULL << (ADC_CHANNEL + SENS_SAR1_EN_PAD_S));
}

__attribute__((always_inline)) inline static void adc_init(void) {
  SANE_ASSERT(!ADC_READY);

  adc_set_bit_width();
  adc_prescale();
  adc_redirect_from_gpio();
  adc_set_attenuation();
  adc_set_software_control();
  adc_set_channel();

#ifndef NDEBUG
  ADC_READY = 1;
#endif // NDEBUG
}

uint16_t adc_poll(void) {
  SANE_ASSERT(ADC_READY);

  do {
  } while (force_32b_read(REG_PTR(SENS_SAR_SLAVE_ADDR1_REG), SENS_MEAS_STATUS_M)); // This register is completely absent from the manual--not even its memory address :_)

  REG(SENS_SAR_MEAS_START1_REG) |= SENS_MEAS1_START_SAR_M;

  do {
  } while (!(REG(SENS_SAR_MEAS_START1_REG) & SENS_MEAS1_DONE_SAR_M));

  // return adc_oneshot_ll_get_raw_result(ADC_UNIT_1);
  // ret_val = HAL_FORCE_READ_U32_REG_FIELD(SENS.sar_meas_start1, meas1_data_sar);
  uint16_t adc_value = (force_32b_read(REG_PTR(SENS_SAR_MEAS_START1_REG), SENS_MEAS1_DATA_SAR) >> SENS_MEAS1_DATA_SAR_S /* which happens to be 0 */);

  REG(SENS_SAR_MEAS_START1_REG) &= ~SENS_MEAS1_START_SAR_M;

  return adc_value;
}

#endif // ADCDUINO_H
