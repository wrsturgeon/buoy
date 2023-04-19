#ifndef BUOY_INCLUDE_UART_H
#define BUOY_INCLUDE_UART_H

#include <stdint.h>

void uart_init(void);
void uart_char(uint8_t);
void uart_string(char const*);

#endif // BUOY_INCLUDE_UART_H
