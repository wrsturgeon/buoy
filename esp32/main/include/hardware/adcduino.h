#ifndef ADCDUINO_H
#define ADCDUINO_H

#include </Users/willsturgeon/esp/esp-idf/components/driver/deprecated/driver/adc.h>
#include <driver/rtc_io.h>
#include <esp_log.h>
#include <hal/adc_ll.h>
#include <hal/gpio_hal.h>
#include <soc/adc_channel.h>
#include <soc/rtc_io_reg.h>
#include <soc/sens_reg.h>

#include <inttypes.h>
#include <stdint.h>

#define ADC_ATTENUATION 3
#define ADC_CLK_DIV 1
#define ADC_BIT_WIDTH 12

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

__attribute__((always_inline)) inline static void dropin_adc1_config_width(void) {
  REG(SENS_SAR_START_FORCE_REG) |= SENS_SAR1_BIT_WIDTH_M; // 0x3 => full 12 bits
  REG(SENS_SAR_READ_CTRL_REG) |= SENS_SAR1_SAMPLE_BIT_M;  // ditto ^
}

__attribute__((always_inline)) inline static void dropin_adc_set_clk_div(uint8_t d) {
  ESP_ERROR_CHECK(adc_set_clk_div(d));
}

__attribute__((always_inline)) inline static void dropin_gpio_config(void) {
  static gpio_config_t const GPIO_CONF = {
      .pin_bit_mask = (1ULL << 36),          /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
      .mode = GPIO_MODE_DISABLE,             /*!< GPIO mode: set input/output mode                     */
      .pull_up_en = GPIO_PULLUP_DISABLE,     /*!< GPIO pull-up                                         */
      .pull_down_en = GPIO_PULLDOWN_DISABLE, /*!< GPIO pull-down                                       */
      .intr_type = GPIO_INTR_DISABLE,
  };
  // ESP_ERROR_CHECK(gpio_config(&GPIO_CONF));

  uint32_t io_reg = 0;
  uint32_t io_num = 0;
  uint8_t input_en = 0;
  uint8_t output_en = 0;
  uint8_t od_en = 0;
  uint8_t pu_en = 0;
  uint8_t pd_en = 0;

  do {
    io_reg = GPIO_PIN_MUX_REG[io_num];

    if ((((1ULL << 36) >> io_num) & BIT(0))) {
      assert(io_reg != (intptr_t)NULL);

      if (rtc_gpio_is_valid_gpio(io_num)) {
        rtc_gpio_deinit(io_num);
      }

      ESP_LOGI("gpio", "GPIO[%" PRIu32 "]| InputEn: %d| OutputEn: %d| OpenDrain: %d| Pullup: %d| Pulldown: %d| Intr:%d ", io_num, input_en, output_en, od_en, pu_en, pd_en, GPIO_CONF.intr_type);
      gpio_set_intr_type(io_num, GPIO_CONF.intr_type);

      if (GPIO_CONF.intr_type) {
        gpio_intr_enable(io_num);
      } else {
        gpio_intr_disable(io_num);
      }

#if SOC_GPIO_SUPPORT_PIN_HYS_FILTER
      printf("HYSTERESIS FILTER SUPPORTED\r\n");
      if (GPIO_CONF.hys_ctrl_mode == GPIO_HYS_SOFT_ENABLE) {
        gpio_hysteresis_enable(io_num);
      } else if (GPIO_CONF.hys_ctrl_mode == GPIO_HYS_SOFT_DISABLE) {
        gpio_hysteresis_disable(io_num);
      } else {
        gpio_hysteresis_by_efuse(io_num);
      }
#endif // SOC_GPIO_SUPPORT_PIN_HYS_FILTER
      /* By default, all the pins have to be configured as GPIO pins. */
      gpio_hal_iomux_func_sel(io_reg, PIN_FUNC_GPIO);
    }
  } while (++io_num < GPIO_PIN_COUNT);
}

__attribute__((always_inline)) inline static void dropin_adc1_config_channel_atten(void) {
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_GPIO36_CHANNEL, ADC_ATTENUATION));
}

__attribute__((always_inline)) inline static uint16_t dropin_adc1_get_raw(void) {
  return adc1_get_raw(ADC1_GPIO36_CHANNEL);
}

uint16_t FUCK(void) {
  dropin_adc1_config_width();
  // dropin_adc_set_clk_div(ADC_CLK_DIV);
  dropin_gpio_config();
  dropin_adc1_config_channel_atten();
  return dropin_adc1_get_raw();
}

#endif // ADCDUINO_H
