#ifndef SANE_ASSERT_H
#define SANE_ASSERT_H

#include <assert.h>

#ifndef NDEBUG
#define SANE_ASSERT(...) assert(__VA_ARGS__)
#else                    // NDEBUG
#define SANE_ASSERT(...) // For whatever reason, ESP32's `assert` processes its argument...
#endif                   // NDEBUG

#endif // SANE_ASSERT_H
