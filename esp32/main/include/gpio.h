#ifndef GPIO_H
#define GPIO_H

// Page 48 (section 4.1): "IO_MUX and GPIO Matrix"
// https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
//
// See p. 52 ("Direct I/O via IO_MUX") for our specific strategy, bypassing the recommended GPIO matrix.
// p. 60 lists each GPIO individually.

#include <soc/io_mux_reg.h>

#include <stdint.h>

// Manual p. 62 (btw, "W1TS/W1TC" = "write 1 to [(S)et/(C)lear]")
// It looks like (although this isn't stated anywhere)
//   using the W1TS/W1TC registers is an atomic operation,
//   while using the register directly is not (but probably faster).
// We'll use the register directly.
#define N_GPIO (40U)
#define GPIO_ENABLE_00_THRU_31 (*((uint32_t volatile* restrict const)0x3FF44020)) // Non-atomic! Manual p. 66 (function) & 62 (address). A.K.A. `GPIO_ENABLE_REG`.
#define GPIO_ENABLE_32_THRU_39 (*((uint32_t volatile* restrict const)0x3FF4402C)) // Non-atomic! Manual p. 67 (function) & 62 (address). A.K.A. `GPIO_ENABLE1_REG`.
#define GPIO_FUNC0_ADDR ((uint32_t volatile* restrict const)0x3FF44530)           // Non-atomic! Manual p. 74 (function) & 63 (address). A.K.A. `GPIO_FUNC0_OUT_SEL_CFG_REG`.
#define GPIO_00_THRU_31 (*((uint32_t volatile* restrict const)0x3FF44004))        // Non-atomic! Manual p. 65 (function) & 62 (address). A.K.A. `GPIO_OUT_REG`.
#define GPIO_32_THRU_39 (*((uint32_t volatile* restrict const)0x3FF44010))        // Non-atomic! Manual p. 66 (function) & 62 (address). A.K.A. `GPIO_OUT1_REG`.

// Optional runtime checks against shooting ourselves in the foot
#ifndef NDEBUG
static uint8_t GPIO_ALREADY_ENABLED[N_GPIO >> 3U] = {0}; // each bit represents one pin
#define GPIO_MARK_ENABLED(...)                                                 \
  do {                                                                         \
    assert(((uint32_t)(__VA_ARGS__)) == (__VA_ARGS__));                        \
    assert((__VA_ARGS__) < N_GPIO);                                            \
    GPIO_ALREADY_ENABLED[(__VA_ARGS__) >> 3U] |= (1ULL << ((__VA_ARGS__)&7U)); \
  } while (0)
#define GPIO_MARK_DISABLED(...)                                                 \
  do {                                                                          \
    assert(((uint32_t)(__VA_ARGS__)) == (__VA_ARGS__));                         \
    assert((__VA_ARGS__) < N_GPIO);                                             \
    GPIO_ALREADY_ENABLED[(__VA_ARGS__) >> 3U] &= ~(1ULL << ((__VA_ARGS__)&7U)); \
  } while (0)
#define GPIO_CHECK_ENABLED(...)                                                       \
  do {                                                                                \
    assert(((uint32_t)(__VA_ARGS__)) == (__VA_ARGS__));                               \
    assert((__VA_ARGS__) < N_GPIO);                                                   \
    assert(GPIO_ALREADY_ENABLED[(__VA_ARGS__) >> 3U] & (1ULL << ((__VA_ARGS__)&7U))); \
  } while (0)
#else // NDEBUG
#define GPIO_MARK_ENABLED(...)
#define GPIO_CHECK_ENABLED(...)
#endif // NDEBUG

#define GPIO_ENABLE_OUTPUT(...) GPIO_ENABLE_OUTPUT_LITERAL(__VA_ARGS__) // expand arguments instead of copying them literally
#define GPIO_ENABLE_OUTPUT_LITERAL(...)                                                                                             \
  do {                                                                                                                              \
    GPIO_MARK_ENABLED(__VA_ARGS__);                                                                                                 \
    (*((uint32_t volatile* restrict const)(GPIO_FUNC0_ADDR + ((__VA_ARGS__) << 2U)))) |= ((1ULL << 10U) | 256U); /* Manual p. 74 */ \
    (*((uint32_t volatile* restrict const)(GPIO_FUNC0_ADDR + ((__VA_ARGS__) << 2U)))) &= ~0xFF;                  /* ditto: ^^^^^ */ \
    /*                                                                +++++++++++++++++- Reserved                          */       \
    /*                                                                [               ]+++- MCU_SEL (p. 60)                */       \
    /*                                                                [               ][ ]++- FUN_DRV (strength/amps)      */       \
    /*                                                                [               ][ ][]+- FUN_IE (input enable)       */       \
    /*                                                                [               ][ ][]|+- FUN_WPU (pull-up)          */       \
    /*                                                                [               ][ ][]||+- FUN_WPD (pull-down)       */       \
    /*                                                                [               ][ ][]|||++- MCU_DRV (strength/amps) */       \
    /*                                                                [               ][ ][]|||[]+- MCU_IE (input enable)  */       \
    /*                                                                [               ][ ][]|||[]|+- MCU_WPU (pull-up)     */       \
    /*                                                                [               ][ ][]|||[]||+- MCU_WPD (pull-down)  */       \
    /*                                                                [               ][ ][]|||[]|||+- SLP_SEL (sleep en)  */       \
    /*                                                                [               ][ ][]|||[]||||+- MCU_OE (output en) */       \
    /*                                                                [               ][ ][]|||[]|||||                     */       \
    (*((uint32_t volatile* restrict const)IO_MUX_GPIO##__VA_ARGS__##_REG)) = 0b00000000000000000010010100101001; /* Manual p. 76 */ \
    if ((__VA_ARGS__) < 32) {                                                                                                       \
      GPIO_ENABLE_00_THRU_31 |= (1ULL << (__VA_ARGS__));                                                                            \
    } else {                                                                                                                        \
      GPIO_ENABLE_32_THRU_39 |= (1ULL << ((__VA_ARGS__)&31U));                                                                      \
    }                                                                                                                               \
  } while (0)
#define GPIO_DISABLE_OUTPUT(...) GPIO_DISABLE_OUTPUT_LITERAL(__VA_ARGS__) // expand arguments instead of copying them literally
#define GPIO_DISABLE_OUTPUT_LITERAL(...)                                                                                            \
  do {                                                                                                                              \
    GPIO_MARK_DISABLED(__VA_ARGS__);                                                                                                \
    (*((uint32_t volatile* restrict const)(GPIO_FUNC0_ADDR + ((__VA_ARGS__) << 2U)))) |= ((1ULL << 10U) | 256U); /* Manual p. 74 */ \
    (*((uint32_t volatile* restrict const)(GPIO_FUNC0_ADDR + ((__VA_ARGS__) << 2U)))) &= ~0xFF;                  /* ditto: ^^^^^ */ \
    if ((__VA_ARGS__) < 32) {                                                                                                       \
      GPIO_ENABLE_00_THRU_31 &= ~(1ULL << (__VA_ARGS__));                                                                           \
    } else {                                                                                                                        \
      GPIO_ENABLE_32_THRU_39 &= ~(1ULL << ((__VA_ARGS__)&31U));                                                                     \
    }                                                                                                                               \
  } while (0)

#define GPIO_PULL_HI(...)                                  \
  do {                                                     \
    GPIO_CHECK_ENABLED(__VA_ARGS__);                       \
    if ((__VA_ARGS__) < 32) { /* should be compile-time */ \
      GPIO_00_THRU_31 |= (1ULL << (__VA_ARGS__));          \
    } else {                                               \
      GPIO_32_THRU_39 |= (1ULL << ((__VA_ARGS__)&31U));    \
    }                                                      \
  } while (0)
#define GPIO_PULL_LO(...)                                  \
  do {                                                     \
    GPIO_CHECK_ENABLED(__VA_ARGS__);                       \
    if ((__VA_ARGS__) < 32) { /* should be compile-time */ \
      GPIO_00_THRU_31 &= ~(1ULL << (__VA_ARGS__));         \
    } else {                                               \
      GPIO_32_THRU_39 &= ~(1ULL << ((__VA_ARGS__)&31U));   \
    }                                                      \
  } while (0)

#endif // GPIO_H
