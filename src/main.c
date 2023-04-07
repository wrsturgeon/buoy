#include "graphics.h"
#include "uart.h"

#include <avr/interrupt.h>

#define LG_ADCN 6U // should always be a power of 2

static uint16_t adchist[1U << LG_ADCN];
static uint8_t adcvis = 0;
static uint16_t heartbpm = 0;

ISR(ADC_vect) {
  static uint8_t adci = 0;
  adchist[adci++] = ADCL + (((uint16_t)(ADCH)) << 8U);
  if (adci == (1U << LG_ADCN)) {
    adci = 0;
    adcvis = 1;
  }
}

int main(void) {

  // Disable interrupts during the crucial initialization period
  cli();

  // Initialize serial communication
  uart_init();

  // ADC
  PRR &= ~(1U << PRADC);
  ADMUX |= (1U << REFS0);
  ADMUX &= ~((1U << REFS1) | (1U << MUX0) | (1U << MUX1) | (1U << MUX2) | (1U << MUX3));
  ADCSRA |= (1U << ADPS0) | (1U << ADPS1) | (1U << ADPS2) | (1U << ADATE);
  ADCSRB &= ~((1U << ADTS0) | (1U << ADTS1) | (1U << ADTS2));
  DIDR0 |= (1U << ADC0D);
  ADCSRA |= (1U << ADEN) | (1U << ADIE) | (1U << ADSC);

  // Re-enable interrupts
  sei();

  init_graphics();

  uint16_t adcval;
  do {
    //    do {
    //    } while (!adcvis);
    adcvis = adcval = 0;
    for (uint8_t i = 0; i != (1U << LG_ADCN); ++i) { adcval += adchist[i]; };
    vis_pulse(adcval >>= LG_ADCN);
    // uart_char('0' + (adcval / 1000U));
    // uart_char('0' + ((adcval / 100U) % 10U));
    // uart_char('0' + ((adcval / 10U) % 10U));
    // uart_char('0' + (adcval % 10U));
    // uart_char('\r');
    // uart_char('\n');
    update_bpm(heartbpm++);
  } while (1);
}
