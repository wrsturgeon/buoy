#ifndef CIRCLEBUFFER_H
#define CIRCLEBUFFER_H

#include <stdint.h>

#define MAKE_CIRCLEBUFFER(T, N)                                                                                       \
  typedef struct CircleBuffer_##T##_##N {                                                                             \
    T##_t values[N];                                                                                                  \
    uint8_t i;                                                                                                        \
  } CircleBuffer_##T##_##N;                                                                                           \
  __attribute__((always_inline)) inline static CircleBuffer_##T##_##N cbuf_new_##T##_##N(void) {                      \
    CircleBuffer_##T##_##N b;                                                                                         \
    b.i = 0;                                                                                                          \
    return b;                                                                                                         \
  }                                                                                                                   \
  __attribute__((always_inline)) inline static T##_t cbuf_peek_##T##_##N(CircleBuffer_##T##_##N const* b) {           \
    return b->values[b->i];                                                                                           \
  }                                                                                                                   \
  __attribute__((always_inline)) inline static T##_t cbuf_get_##T##_##N(uint8_t i, CircleBuffer_##T##_##N const* b) { \
    return b->values[i]; /* unchecked */                                                                              \
  }                                                                                                                   \
  __attribute__((always_inline)) inline static void cbuf_push_##T##_##N(T##_t v, CircleBuffer_##T##_##N* b) {         \
    b->values[b->i++] = v;                                                                                            \
    if (b->i > (N)) { b->i = 0; }                                                                                     \
  }

MAKE_CIRCLEBUFFER(uint16, 2)
MAKE_CIRCLEBUFFER(uint16, 4)
MAKE_CIRCLEBUFFER(uint16, 8)
MAKE_CIRCLEBUFFER(uint16, 16)
MAKE_CIRCLEBUFFER(uint16, 32)
MAKE_CIRCLEBUFFER(uint16, 64)

#endif // CIRCLEBUFFER_H