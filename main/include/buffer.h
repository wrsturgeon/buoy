#ifndef BUFFER_H
#define BUFFER_H

#include "bitbang/st7735.h"

#include <stdint.h>

#define BUFFER_LENGTH ST7735_H
typedef uint8_t buffer_element_t;
typedef uint8_t buffer_index_t;

typedef struct {
  buffer_element_t v[BUFFER_LENGTH];
  buffer_index_t i;
} buffer_t;

__attribute__((always_inline)) inline static void buffer_push(buffer_element_t const x, buffer_t* const restrict b) {
  b->v[b->i] = x;
  if (BUFFER_LENGTH == ++(b->i)) { b->i = 0; }
}

__attribute__((always_inline)) inline static buffer_element_t buffer_get(buffer_index_t const i, buffer_t const* const restrict b) {
  uint8_t shifted_index = i + b->i;
  if (shifted_index >= BUFFER_LENGTH) { shifted_index -= BUFFER_LENGTH; } // This condition is fine as long as BUFFER_LENGTH <= 128
  return b->v[shifted_index];
}

#endif // BUFFER_H
