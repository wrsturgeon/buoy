#include "circlebuffer.h"
#include "graphics.h"
#include "uart.h"

#include <avr/interrupt.h>
#include <util/delay.h>

#define LG_ADCN 6U // should always be a power of 2
#define LG_HBUF 4U

static uint8_t tcnt1ovf;
static uint16_t adchist[1U << LG_ADCN];
static uint8_t adcvis = 0;
static CircleBuffer_uint16_16 heartbuf;

ISR(TIMER1_OVF_vect) { ++tcnt1ovf; }

ISR(ADC_vect) {
  static uint8_t adci = 0;
  adchist[adci++] = ADCL + (((uint16_t)(ADCH)) << 8U);
  if (adci == (1U << LG_ADCN)) {
    adci = 0;
    adcvis = 1;
  }
}

__attribute__((always_inline)) uint16_t time_since_last_beat_62500sec(void) {
  // Average resting heart rate ~60-100bpm
  // We should be able to cover the vast, vast majority on either tail
  // So let's say the max #seconds between beats is 16 (just under 4bpm!)
  // So (raw output) >> 8 -> seconds
  // `F_CPU` = #clk/sec
  // So we need to handle at least `F_CPU >> 8` clocks since last beat
  // With our current setup, that's 62500L, which fits perfectly into 15.93 bits!
  static uint16_t last; // don't care, wrong whatever it is
  uint16_t const lastlast = last;
  cli();
  uint8_t tcnt = (TCNT1 >> 8);
  sei();
  last = (((uint16_t)(((uint16_t)tcnt1ovf) << 8)) | tcnt); // unit: 256clk; note that 1=16000000clk/sec, so this is also 0.000016sec
  return last - lastlast;
}

__attribute__((always_inline)) void beep(void) {
  PORTD |= (1U << PD2);
  _delay_ms(1);
  PORTD &= ~(1U << PD2);
}

int main(void) {

  // Disable interrupts during the crucial initialization period
  cli();

  // Initialize serial communication
  uart_init();

  // Timer 1 overflow interrupt
  TCCR1A = 0U;
  TCCR1B |= (1U << CS10);
  TIMSK1 |= (1U << TOIE1);

  // Speaker (D2)
  DDRD |= (1U << DDD2);
  PORTD &= ~(1U << PD2);

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
    do {
    } while (!adcvis);
    adcvis = adcval = 0;
    for (uint8_t i = 0; i != (1U << LG_ADCN); ++i) { adcval += adchist[i]; };
    if (is_heartbeat(adcval >>= LG_ADCN) /* also displays */) {
      cbuf_push_uint16_16(time_since_last_beat_62500sec(), &heartbuf);
      uint16_t mean;
      {
        uint32_t sum = 0;
        for (uint16_t i = 0; i < (1U << LG_HBUF); ++i) { sum += heartbuf.values[i]; }
        mean = (sum >> LG_HBUF);
      }
      beep();
      // We want an operation s.t.
      // (X) / (sec / 62,500) = beats / minute
      // 62,500 X / sec = beats / (60 sec)
      // 3,750,000 X = beats
      // X = beats / 3,750,000
      // and 3,750,000 >> 6 fits within 16b (58,593.75)
      update_bpm(58594U / (mean >> 6));
    }
    // uart_char('0' + (adcval / 1000U));
    // uart_char('0' + ((adcval / 100U) % 10U));
    // uart_char('0' + ((adcval / 10U) % 10U));
    // uart_char('0' + (adcval % 10U));
    // uart_char('\r');
    // uart_char('\n');
  } while (1);
}
