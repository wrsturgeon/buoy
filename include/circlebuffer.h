#ifndef BUOY_INCLUDE_CIRCLEBUFFER_H
#define BUOY_INCLUDE_CIRCLEBUFFER_H

#include <stdint.h>

#define CIRCLEBUFFER_FUNCTIONS(T, N)                                                              \
  __attribute__((always_inline)) inline CircleBuffer_##T##_##N new_##T##_##N##_buffer(void) {     \
    CircleBuffer_##T##_##N b;                                                                     \
    b.i = 0;                                                                                      \
    return b;                                                                                     \
  }                                                                                               \
  __attribute__((always_inline)) inline void push_##T##_##N(T##_t v, CircleBuffer_##T##_##N* b) { \
    b->values[b->i++] = v;                                                                        \
    if (b->i > (N)) { b->i = 0; }                                                                 \
  }

// Use when N < 256 (special case when N=256).
#define MAKE_CIRCLEBUFFER_8B(T, N)        \
  typedef struct CircleBuffer_##T##_##N { \
    T##_t values[N];                      \
    uint8_t i;                            \
  } CircleBuffer_##T##_##N;               \
  CIRCLEBUFFER_FUNCTIONS(T, N)

// Use when N > 256.
#define MAKE_CIRCLEBUFFER_16B(T, N)       \
  typedef struct CircleBuffer_##T##_##N { \
    T##_t values[N];                      \
    uint16_t i;                           \
  } CircleBuffer_##T##_##N;               \
  CIRCLEBUFFER_FUNCTIONS(T, N)

MAKE_CIRCLEBUFFER_8B(uint16, 128)

#endif // BUOY_INCLUDE_CIRCLEBUFFER_H
