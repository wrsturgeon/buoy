#ifndef ADCDUINO_H
#define ADCDUINO_H

#include </Users/willsturgeon/esp/esp-idf/components/driver/deprecated/driver/adc.h>
#include <hal/gpio_hal.h>
#include <soc/adc_channel.h>
#include <soc/dac_channel.h>
#include <soc/rtc_io_reg.h>
#include <soc/sens_reg.h>
#include <soc/touch_sensor_channel.h>

#include <stdint.h>

#define ADC_ATTENUATION 3
#define ADC_CLK_DIV 1

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

int const touch_sensor_channel_io_map[] = {
    TOUCH_PAD_NUM0_GPIO_NUM,
    TOUCH_PAD_NUM1_GPIO_NUM,
    TOUCH_PAD_NUM2_GPIO_NUM,
    TOUCH_PAD_NUM3_GPIO_NUM,
    TOUCH_PAD_NUM4_GPIO_NUM,
    TOUCH_PAD_NUM5_GPIO_NUM,
    TOUCH_PAD_NUM6_GPIO_NUM,
    TOUCH_PAD_NUM7_GPIO_NUM,
    TOUCH_PAD_NUM8_GPIO_NUM,
    TOUCH_PAD_NUM9_GPIO_NUM};

static adc_attenuation_t __pin_attenuation[SOC_GPIO_PIN_COUNT];

uint16_t FUCK(void) {

  //  <<< analogReadResolution(12);
  ESP_ERROR_CHECK(adc1_config_width(12));

  //  <<< analogRead(36);
  //    <<< __adcAttachPin(36);
  //      <<< __analogInit();
  //        <<< __analogSetClockDiv(1);
  adc_set_clk_div(ADC_CLK_DIV);
  for (int i = 0; i < SOC_GPIO_PIN_COUNT; i++) {
    __pin_attenuation[i] = ADC_ATTENDB_MAX;
  }

  //    <<< int8_t pad = digitalPinToTouchChannel(36);
  int8_t pad = -1;
  for (uint8_t i = 0; i < SOC_TOUCH_SENSOR_NUM; i++) {
    if (touch_sensor_channel_io_map[i] == 36) {
      pad = i;
      break;
    }
  }

  if (pad >= 0) {
    uint32_t touch = READ_PERI_REG(SENS_SAR_TOUCH_ENABLE_REG);
    if (touch & (1 << pad)) {
      touch &= ~((1 << (pad + SENS_TOUCH_PAD_OUTEN2_S)) | (1 << (pad + SENS_TOUCH_PAD_OUTEN1_S)) | (1 << (pad + SENS_TOUCH_PAD_WORKEN_S)));
      WRITE_PERI_REG(SENS_SAR_TOUCH_ENABLE_REG, touch);
    }
  }
#if SOC_DAC_SUPPORTED
  else if (36 == DAC_CHANNEL_1_GPIO_NUM) {
    CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_XPD_DAC | RTC_IO_PDAC1_DAC_XPD_FORCE); // stop dac1
  } else if (36 == DAC_CHANNEL_2_GPIO_NUM) {
    CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC2_REG, RTC_IO_PDAC2_XPD_DAC | RTC_IO_PDAC2_DAC_XPD_FORCE); // stop dac2
  }
#endif

  //    <<< pinMode(36, ANALOG);
  gpio_hal_context_t gpiohal;
  gpiohal.dev = GPIO_LL_GET_HW(GPIO_PORT_0);
  gpio_config_t conf = {
      .pin_bit_mask = (1ULL << 36),              /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
      .mode = GPIO_MODE_DISABLE,                 /*!< GPIO mode: set input/output mode                     */
      .pull_up_en = GPIO_PULLUP_DISABLE,         /*!< GPIO pull-up                                         */
      .pull_down_en = GPIO_PULLDOWN_DISABLE,     /*!< GPIO pull-down                                       */
      .intr_type = gpiohal.dev->pin[36].int_type /*!< GPIO interrupt type - previously set                 */
  };
#if (ANALOG < 0x20) // io
  conf.mode = ANALOG & (INPUT | OUTPUT);
#if (ANALOG & OPEN_DRAIN)
  conf.mode |= GPIO_MODE_DEF_OD;
#endif
#if (ANALOG & PULLUP)
  conf.pull_up_en = GPIO_PULLUP_ENABLE;
#endif
#if (ANALOG & PULLDOWN)
  conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
#endif
#endif // ANALOG < 0x20
  ESP_ERROR_CHECK(gpio_config(&conf));

  //    <<< __analogSetPinAttenuation(36, (__pin_attenuation[36] != ADC_ATTENDB_MAX) ? __pin_attenuation[36] : __analogAttenuation);
  adc_attenuation_t const attenuation = ((__pin_attenuation[36] != ADC_ATTENDB_MAX) ? __pin_attenuation[36] : ADC_ATTENUATION);
#if (ADC1_GPIO36_CHANNEL > (SOC_ADC_MAX_CHANNEL_NUM - 1))
  adc2_config_channel_atten(ADC1_GPIO36_CHANNEL - SOC_ADC_MAX_CHANNEL_NUM, attenuation);
#else
  adc1_config_channel_atten(ADC1_GPIO36_CHANNEL, attenuation);
#endif
  //    <<< __analogInit();
  for (int i = 0; i < SOC_GPIO_PIN_COUNT; i++) {
    __pin_attenuation[i] = ADC_ATTENDB_MAX;
  }

  if ((__pin_attenuation[36] != ADC_ATTENDB_MAX) || (attenuation != ADC_ATTENUATION)) {
    __pin_attenuation[36] = attenuation;
  }

  int value;
#if (ADC1_GPIO36_CHANNEL > (SOC_ADC_MAX_CHANNEL_NUM - 1))
  esp_err_t r;
  switch (r = adc2_get_raw(ADC1_GPIO36_CHANNEL - SOC_ADC_MAX_CHANNEL_NUM, __analogWidth, &value)) {
  case ESP_OK: break;
  case ESP_ERR_INVALID_STATE: log_e("GPIO%u: %s: ADC2 not initialized yet.", 36, esp_err_to_name(r)); return -1;
  case ESP_ERR_TIMEOUT: log_e("GPIO%u: %s: ADC2 is in use by Wi-Fi. Please see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html#adc-limitations for more info", 36, esp_err_to_name(r)); return -1;
  default: log_e("GPIO%u: %s", 36, esp_err_to_name(r)); return -1;
  }
#else
  value = adc1_get_raw(ADC1_GPIO36_CHANNEL);
#endif
  //  <<< return mapResolution(value);
  return value;
}

#endif // ADCDUINO_H
