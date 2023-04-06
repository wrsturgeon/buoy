#include <avr/interrupt.h>

ISR(ADC_vect) {
  // TODO
}

int main(void) {

  // Disable interrupts during the crucial initialization period
  cli();

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

  // TODO
}
