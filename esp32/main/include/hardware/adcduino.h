#ifndef ADCDUINO_H
#define ADCDUINO_H

#include "stringify.h"

#include <../deprecated/adc1_private.h>
#include <../deprecated/driver/adc.h>
#include </Users/willsturgeon/esp/esp-idf/components/driver/deprecated/driver/adc.h>
#include <driver/rtc_io.h>
#include <esp_log.h>
#include <hal/adc_ll.h>
#include <hal/gpio_hal.h>
#include <soc/adc_channel.h>
#include <soc/io_mux_reg.h>
#include <soc/rtc_io_channel.h>
#include <soc/rtc_io_periph.h>
#include <soc/rtc_io_reg.h>
#include <soc/sens_reg.h>

#include <inttypes.h>
#include <stdint.h>

#define ADC_ATTENUATION 3
#define ADC_CLK_DIV 1
#define ADC_BIT_WIDTH 12
#define ADC_PIN 36
#define ADC_CHANNEL PASTE(PASTE(ADC1_GPIO, ADC_PIN), _CHANNEL)
#define ADC_IO_REG PASTE(PASTE(IO_MUX_GPIO, ADC_PIN), _REG)

#define RTC_IO_CHANNEL PASTE(PASTE(RTCIO_GPIO, ADC_PIN), _CHANNEL)

// GPIO FUNCTIONS
#define INPUT 0x01
// Changed OUTPUT from 0x02 to behave the same as Arduino pinMode(pin,OUTPUT)
// where you can read the state of pin even when it is set as OUTPUT
#define OUTPUT 0x03
#define PULLUP 0x04
#define INPUT_PULLUP 0x05
#define PULLDOWN 0x08
#define INPUT_PULLDOWN 0x09
#define OPEN_DRAIN 0x10
#define OUTPUT_OPEN_DRAIN 0x12
#define ANALOG 0xC0

typedef enum {
  ADC_0db,
  ADC_2_5db,
  ADC_6db,
  ADC_11db,
  ADC_ATTENDB_MAX
} adc_attenuation_t;

extern rtc_io_desc_t const rtc_io_desc[SOC_RTCIO_PIN_COUNT];

__attribute__((always_inline)) inline static void dropin_adc1_config_width(void) {
  REG(SENS_SAR_START_FORCE_REG) |= SENS_SAR1_BIT_WIDTH_M; // 0x3 => full 12 bits
  REG(SENS_SAR_READ_CTRL_REG) |= SENS_SAR1_SAMPLE_BIT_M;  // ditto ^
}

__attribute__((always_inline)) inline static void dropin_adc_set_clk_div(uint8_t d) {
  ESP_ERROR_CHECK(adc_set_clk_div(d));
}

__attribute__((always_inline)) inline static void dropin_rtc_gpio_deinit(void) {
  REG(rtc_io_desc[RTC_IO_CHANNEL].reg) &= ~rtc_io_desc[RTC_IO_CHANNEL].mux;
}

__attribute__((always_inline)) inline static void dropin_gpio_set_intr_type(void) {
  // gpio_set_intr_type(ADC_PIN, GPIO_INTR_DISABLE);

  // #define gpio_hal_set_intr_type(hal, gpio_num, intr_type) gpio_ll_set_intr_type((hal)->dev, gpio_num, intr_type)
  //   gpio_hal_set_intr_type(gpio_context.gpio_hal, ADC_PIN, GPIO_INTR_DISABLE);
  // gpio_ll_set_intr_type(gpio_context.gpio_hal->dev, ADC_PIN, GPIO_INTR_DISABLE);
  // gpio_context.gpio_hal->dev->pin[ADC_PIN].int_type = GPIO_INTR_DISABLE;
  REG(PASTE(PASTE(GPIO_PIN, ADC_PIN), _REG)) &= ~PASTE(PASTE(GPIO_PIN, ADC_PIN), _INT_TYPE_M);

  //   gpio_context.isr_clr_on_entry_mask &= ~(1ULL << ADC_PIN);
  //   gpio_hal_clear_intr_status_bit(gpio_context.gpio_hal, ADC_PIN);
  // #define gpio_hal_clear_intr_status_bit(hal, gpio_num) (((gpio_num) < 32) ? gpio_ll_clear_intr_status((hal)->dev, 1 << gpio_num) : gpio_ll_clear_intr_status_high((hal)->dev, 1 << (gpio_num - 32)))
  //   HAL_FORCE_MODIFY_U32_REG_FIELD(hw->status1_w1tc, intr_st, mask);
}

__attribute__((always_inline)) inline static void dropin_gpio_intr_disable(void) {
  REG(GPIO_PIN36_REG) &= ~GPIO_PIN36_INT_ENA_M;
#if (ADC_PIN < 32)
  REG(GPIO_STATUS_W1TC_REG) = (1ULL << ADC_PIN);
#else
  REG(GPIO_STATUS1_W1TC_REG) = (1ULL << (ADC_PIN - 32U));
#endif
}

__attribute__((always_inline)) inline static void dropin_gpio_hal_iomux_func_sel(void) {
  REG(ADC_IO_REG) &= ~MCU_SEL_M;
  REG(ADC_IO_REG) |= (PIN_FUNC_GPIO << MCU_SEL_S);
}

__attribute__((always_inline)) inline static void dropin_gpio_config(void) {
  dropin_rtc_gpio_deinit();
  dropin_gpio_set_intr_type();
  dropin_gpio_intr_disable();
  dropin_gpio_hal_iomux_func_sel();
}

__attribute__((always_inline)) inline static void dropin_adc1_config_channel_atten(void) {
  ESP_ERROR_CHECK(adc1_config_channel_atten(PASTE(PASTE(ADC1_GPIO, ADC_PIN), _CHANNEL), ADC_ATTENUATION));
}

__attribute__((always_inline)) inline static uint16_t dropin_adc1_get_raw(void) {
  int adc_value;
  adc1_rtc_mode_acquire();

#if SOC_ADC_CALIBRATION_V1_SUPPORTED
  adc_atten_t atten = adc_ll_get_atten(ADC_UNIT_1, ADC_CHANNEL);
  adc_set_hw_calibration_code(ADC_UNIT_1, atten);
#endif // SOC_ADC_CALIBRATION_V1_SUPPORTED

  // SARADC1_ENTER();
#ifdef CONFIG_IDF_TARGET_ESP32
  adc_ll_hall_disable(); // Disable other peripherals.
  adc_ll_amp_disable();  // Currently the LNA is not open, close it by default.
#endif
  adc_ll_set_controller(ADC_UNIT_1, ADC_LL_CTRL_RTC); // Set controller
  adc_oneshot_ll_set_channel(ADC_UNIT_1, ADC_CHANNEL);

  // adc_hal_convert(ADC_UNIT_1, ADC_CHANNEL, clk_src_freq_hz, &adc_value); // Start conversion, For ADC1, the data always valid.
  adc_oneshot_ll_clear_event(ADC_LL_EVENT_ADC1_ONESHOT_DONE);
  adc_oneshot_ll_disable_all_unit();
  adc_oneshot_ll_enable(ADC_UNIT_1);
  adc_oneshot_ll_set_channel(ADC_UNIT_1, ADC_CHANNEL);
  { // adc_hal_onetime_start(ADC_UNIT_1, clk_src_freq_hz);
#if SOC_ADC_DIG_CTRL_SUPPORTED && !SOC_ADC_RTC_CTRL_SUPPORTED
    printf("NOTE THAT (SOC_ADC_DIG_CTRL_SUPPORTED && !SOC_ADC_RTC_CTRL_SUPPORTED)\r\n");
    (void)ADC_UNIT_1;
    /**
     * There is a hardware limitation. If the APB clock frequency is high, the step of this reg signal: ``onetime_start`` may not be captured by the
     * ADC digital controller (when its clock frequency is too slow). A rough estimate for this step should be at least 3 ADC digital controller
     * clock cycle.
     */
    uint32_t digi_clk = clk_src_freq_hz / (ADC_LL_CLKM_DIV_NUM_DEFAULT + ADC_LL_CLKM_DIV_A_DEFAULT / ADC_LL_CLKM_DIV_B_DEFAULT + 1);
    // Convert frequency to time (us). Since decimals are removed by this division operation. Add 1 here in case of the fact that delay is not enough.
    uint32_t delay = (1000 * 1000) / digi_clk + 1;
    // 3 ADC digital controller clock cycle
    delay = delay * 3;
    // This coefficient (8) is got from test, and verified from DT. When digi_clk is not smaller than ``APB_CLK_FREQ/8``, no delay is needed.
    if (digi_clk >= APB_CLK_FREQ / 8) {
      delay = 0;
    }

    adc_oneshot_ll_start(false);
    esp_rom_delay_us(delay);
    adc_oneshot_ll_start(true);

    // No need to delay here. Becuase if the start signal is not seen, there won't be a done intr.
#else
    adc_oneshot_ll_start(ADC_UNIT_1);
#endif
  }
  do {
  } while (adc_oneshot_ll_get_event(ADC_LL_EVENT_ADC1_ONESHOT_DONE) != true);
  adc_value = adc_oneshot_ll_get_raw_result(ADC_UNIT_1);
  adc_oneshot_ll_disable_all_unit();

  // SARADC1_EXIT();

  adc1_lock_release();
  return adc_value;
}

uint16_t FUCK(void) {
  dropin_adc1_config_width();
  // dropin_adc_set_clk_div(ADC_CLK_DIV);
  dropin_gpio_config();
  dropin_adc1_config_channel_atten();
  // NOTE: you can apparently safely delete everything above this line, but I haven't tested after unplugging
  return dropin_adc1_get_raw();
}

#endif // ADCDUINO_H
