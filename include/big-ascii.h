#ifndef BUOY_INCLUDE_BIG_ASCII_H
#define BUOY_INCLUDE_BIG_ASCII_H

#include <stdint.h>

// All these are 7x9 "characters" (some are icons like hearts).
// In general, x & y are flipped.

#define BIGASCII_W 7
#define BIGASCII_H 9

void big_integer(uint8_t i, uint8_t x, uint8_t y, uint16_t color);

typedef void (*big_ascii_fn)(uint8_t, uint8_t, uint16_t);

#define MAKE_BIG(FNNAME) void big_##FNNAME(uint8_t x, uint8_t y, uint16_t color)
MAKE_BIG(heart);
MAKE_BIG(question);
MAKE_BIG(lparen);
MAKE_BIG(rparen);
MAKE_BIG(0);
MAKE_BIG(1);
MAKE_BIG(2);
MAKE_BIG(3);
MAKE_BIG(4);
MAKE_BIG(5);
MAKE_BIG(6);
MAKE_BIG(7);
MAKE_BIG(8);
MAKE_BIG(9);
MAKE_BIG(B);
MAKE_BIG(P);
MAKE_BIG(M);
#undef MAKE_BIG

#endif // BUOY_INCLUDE_BIG_ASCII_H
