#include "uart.h"
#include <avr/io.h>

#define BAUDRATE 9600
#define PRESCALE (((F_CPU / (BAUDRATE * 16UL))) - 1U)

void uart_init(void) {
  // Baud rate
  UBRR0H = (uint8_t)(PRESCALE >> 8);
  UBRR0L = (uint8_t)(PRESCALE);
  // Enable receiver and transmitter
  UCSR0B = (1U << RXEN0) | (1U << TXEN0);
  // 2 stop bits, 8 data bits
  UCSR0C = (1U << UCSZ01) | (1U << UCSZ00); // 8 data bits
  UCSR0C |= (1U << USBS0);                  // 2 stop bits
}

void uart_char(uint8_t data) {
  do {
  } while (!(UCSR0A & (1U << UDRE0)));
  UDR0 = data;
}

void uart_string(char const* cstr) {
  if (!cstr) {
    cstr = "\r\nERROR: `serial_cstr` passed a null pointer!\r\n";
    goto nonnull;
  }
  if (*cstr) { // to handle the empty string
    do {
    nonnull:
      uart_char(*cstr);
    } while (*++cstr);
  }
}
