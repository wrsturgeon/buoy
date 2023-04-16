#ifndef ADCDUINO_H
#define ADCDUINO_H

#include </Users/willsturgeon/esp/esp-idf/components/driver/deprecated/driver/adc.h>
#include <hal/gpio_hal.h>
#include <soc/adc_channel.h>
#include <soc/rtc_io_reg.h>
#include <soc/sens_reg.h>

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

static gpio_config_t const GPIO_CONF = {
    .pin_bit_mask = (1ULL << 36),          /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    .mode = GPIO_MODE_DISABLE,             /*!< GPIO mode: set input/output mode                     */
    .pull_up_en = GPIO_PULLUP_DISABLE,     /*!< GPIO pull-up                                         */
    .pull_down_en = GPIO_PULLDOWN_DISABLE, /*!< GPIO pull-down                                       */
    .intr_type = GPIO_INTR_DISABLE,
};

uint16_t FUCK(void) {
  ESP_ERROR_CHECK(adc1_config_width(12));
  ESP_ERROR_CHECK(adc_set_clk_div(ADC_CLK_DIV));

  //    <<< pinMode(36, ANALOG);
  ESP_ERROR_CHECK(gpio_config(&GPIO_CONF));

  adc1_config_channel_atten(ADC1_GPIO36_CHANNEL, ADC_ATTENUATION);

  return adc1_get_raw(ADC1_GPIO36_CHANNEL);
}

#endif // ADCDUINO_H