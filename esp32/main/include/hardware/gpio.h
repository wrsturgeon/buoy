#ifndef GPIO_H
#define GPIO_H

// Page 48 (section 4.1): "IO_MUX and GPIO Matrix"
// https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
//
// See p. 52 ("Direct I/O via IO_MUX") for our specific strategy, bypassing the recommended GPIO matrix.
// p. 60 lists each GPIO individually.

#include "sane-assert.h"

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
    SANE_ASSERT(((uint32_t)(__VA_ARGS__)) == (__VA_ARGS__));                   \
    SANE_ASSERT((__VA_ARGS__) < N_GPIO);                                       \
    GPIO_ALREADY_ENABLED[(__VA_ARGS__) >> 3U] |= (1ULL << ((__VA_ARGS__)&7U)); \
  } while (0)
#define GPIO_MARK_DISABLED(...)                                                 \
  do {                                                                          \
    SANE_ASSERT(((uint32_t)(__VA_ARGS__)) == (__VA_ARGS__));                    \
    SANE_ASSERT((__VA_ARGS__) < N_GPIO);                                        \
    GPIO_ALREADY_ENABLED[(__VA_ARGS__) >> 3U] &= ~(1ULL << ((__VA_ARGS__)&7U)); \
  } while (0)
#define GPIO_CHECK_ENABLED(...)                                                            \
  do {                                                                                     \
    _Static_assert(((uint32_t)(__VA_ARGS__)) == (__VA_ARGS__), "GPIO pin > 65536?!");      \
    _Static_assert((__VA_ARGS__) < N_GPIO, "GPIO pin > N_GPIO");                           \
    SANE_ASSERT(GPIO_ALREADY_ENABLED[(__VA_ARGS__) >> 3U] & (1ULL << ((__VA_ARGS__)&7U))); \
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

#define GPIO_PULL_0_LO() (GPIO_00_THRU_31 &= ~(1ULL << 0U))
#define GPIO_PULL_0_HI() (GPIO_00_THRU_31 |= (1ULL << 0U))
#define GPIO_PULL_1_LO() (GPIO_00_THRU_31 &= ~(1ULL << 1U))
#define GPIO_PULL_1_HI() (GPIO_00_THRU_31 |= (1ULL << 1U))
#define GPIO_PULL_2_LO() (GPIO_00_THRU_31 &= ~(1ULL << 2U))
#define GPIO_PULL_2_HI() (GPIO_00_THRU_31 |= (1ULL << 2U))
#define GPIO_PULL_3_LO() (GPIO_00_THRU_31 &= ~(1ULL << 3U))
#define GPIO_PULL_3_HI() (GPIO_00_THRU_31 |= (1ULL << 3U))
#define GPIO_PULL_4_LO() (GPIO_00_THRU_31 &= ~(1ULL << 4U))
#define GPIO_PULL_4_HI() (GPIO_00_THRU_31 |= (1ULL << 4U))
#define GPIO_PULL_5_LO() (GPIO_00_THRU_31 &= ~(1ULL << 5U))
#define GPIO_PULL_5_HI() (GPIO_00_THRU_31 |= (1ULL << 5U))
#define GPIO_PULL_6_LO() (GPIO_00_THRU_31 &= ~(1ULL << 6U))
#define GPIO_PULL_6_HI() (GPIO_00_THRU_31 |= (1ULL << 6U))
#define GPIO_PULL_7_LO() (GPIO_00_THRU_31 &= ~(1ULL << 7U))
#define GPIO_PULL_7_HI() (GPIO_00_THRU_31 |= (1ULL << 7U))
#define GPIO_PULL_8_LO() (GPIO_00_THRU_31 &= ~(1ULL << 8U))
#define GPIO_PULL_8_HI() (GPIO_00_THRU_31 |= (1ULL << 8U))
#define GPIO_PULL_9_LO() (GPIO_00_THRU_31 &= ~(1ULL << 9U))
#define GPIO_PULL_9_HI() (GPIO_00_THRU_31 |= (1ULL << 9U))
#define GPIO_PULL_10_LO() (GPIO_00_THRU_31 &= ~(1ULL << 10U))
#define GPIO_PULL_10_HI() (GPIO_00_THRU_31 |= (1ULL << 10U))
#define GPIO_PULL_11_LO() (GPIO_00_THRU_31 &= ~(1ULL << 11U))
#define GPIO_PULL_11_HI() (GPIO_00_THRU_31 |= (1ULL << 11U))
#define GPIO_PULL_12_LO() (GPIO_00_THRU_31 &= ~(1ULL << 12U))
#define GPIO_PULL_12_HI() (GPIO_00_THRU_31 |= (1ULL << 12U))
#define GPIO_PULL_13_LO() (GPIO_00_THRU_31 &= ~(1ULL << 13U))
#define GPIO_PULL_13_HI() (GPIO_00_THRU_31 |= (1ULL << 13U))
#define GPIO_PULL_14_LO() (GPIO_00_THRU_31 &= ~(1ULL << 14U))
#define GPIO_PULL_14_HI() (GPIO_00_THRU_31 |= (1ULL << 14U))
#define GPIO_PULL_15_LO() (GPIO_00_THRU_31 &= ~(1ULL << 15U))
#define GPIO_PULL_15_HI() (GPIO_00_THRU_31 |= (1ULL << 15U))
#define GPIO_PULL_16_LO() (GPIO_00_THRU_31 &= ~(1ULL << 16U))
#define GPIO_PULL_16_HI() (GPIO_00_THRU_31 |= (1ULL << 16U))
#define GPIO_PULL_17_LO() (GPIO_00_THRU_31 &= ~(1ULL << 17U))
#define GPIO_PULL_17_HI() (GPIO_00_THRU_31 |= (1ULL << 17U))
#define GPIO_PULL_18_LO() (GPIO_00_THRU_31 &= ~(1ULL << 18U))
#define GPIO_PULL_18_HI() (GPIO_00_THRU_31 |= (1ULL << 18U))
#define GPIO_PULL_19_LO() (GPIO_00_THRU_31 &= ~(1ULL << 19U))
#define GPIO_PULL_19_HI() (GPIO_00_THRU_31 |= (1ULL << 19U))
#define GPIO_PULL_20_LO() (GPIO_00_THRU_31 &= ~(1ULL << 20U))
#define GPIO_PULL_20_HI() (GPIO_00_THRU_31 |= (1ULL << 20U))
#define GPIO_PULL_21_LO() (GPIO_00_THRU_31 &= ~(1ULL << 21U))
#define GPIO_PULL_21_HI() (GPIO_00_THRU_31 |= (1ULL << 21U))
#define GPIO_PULL_22_LO() (GPIO_00_THRU_31 &= ~(1ULL << 22U))
#define GPIO_PULL_22_HI() (GPIO_00_THRU_31 |= (1ULL << 22U))
#define GPIO_PULL_23_LO() (GPIO_00_THRU_31 &= ~(1ULL << 23U))
#define GPIO_PULL_23_HI() (GPIO_00_THRU_31 |= (1ULL << 23U))
#define GPIO_PULL_24_LO() (GPIO_00_THRU_31 &= ~(1ULL << 24U))
#define GPIO_PULL_24_HI() (GPIO_00_THRU_31 |= (1ULL << 24U))
#define GPIO_PULL_25_LO() (GPIO_00_THRU_31 &= ~(1ULL << 25U))
#define GPIO_PULL_25_HI() (GPIO_00_THRU_31 |= (1ULL << 25U))
#define GPIO_PULL_26_LO() (GPIO_00_THRU_31 &= ~(1ULL << 26U))
#define GPIO_PULL_26_HI() (GPIO_00_THRU_31 |= (1ULL << 26U))
#define GPIO_PULL_27_LO() (GPIO_00_THRU_31 &= ~(1ULL << 27U))
#define GPIO_PULL_27_HI() (GPIO_00_THRU_31 |= (1ULL << 27U))
#define GPIO_PULL_28_LO() (GPIO_00_THRU_31 &= ~(1ULL << 28U))
#define GPIO_PULL_28_HI() (GPIO_00_THRU_31 |= (1ULL << 28U))
#define GPIO_PULL_29_LO() (GPIO_00_THRU_31 &= ~(1ULL << 29U))
#define GPIO_PULL_29_HI() (GPIO_00_THRU_31 |= (1ULL << 29U))
#define GPIO_PULL_30_LO() (GPIO_00_THRU_31 &= ~(1ULL << 30U))
#define GPIO_PULL_30_HI() (GPIO_00_THRU_31 |= (1ULL << 30U))
#define GPIO_PULL_31_LO() (GPIO_00_THRU_31 &= ~(1ULL << 31U))
#define GPIO_PULL_31_HI() (GPIO_00_THRU_31 |= (1ULL << 31U))

#define GPIO_PULL_32_LO() (GPIO_32_THRU_39 &= ~(1ULL << 0U))
#define GPIO_PULL_32_HI() (GPIO_32_THRU_39 |= (1ULL << 0U))
#define GPIO_PULL_33_LO() (GPIO_32_THRU_39 &= ~(1ULL << 1U))
#define GPIO_PULL_33_HI() (GPIO_32_THRU_39 |= (1ULL << 1U))
#define GPIO_PULL_34_LO() (GPIO_32_THRU_39 &= ~(1ULL << 2U))
#define GPIO_PULL_34_HI() (GPIO_32_THRU_39 |= (1ULL << 2U))
#define GPIO_PULL_35_LO() (GPIO_32_THRU_39 &= ~(1ULL << 3U))
#define GPIO_PULL_35_HI() (GPIO_32_THRU_39 |= (1ULL << 3U))
#define GPIO_PULL_36_LO() (GPIO_32_THRU_39 &= ~(1ULL << 4U))
#define GPIO_PULL_36_HI() (GPIO_32_THRU_39 |= (1ULL << 4U))
#define GPIO_PULL_37_LO() (GPIO_32_THRU_39 &= ~(1ULL << 5U))
#define GPIO_PULL_37_HI() (GPIO_32_THRU_39 |= (1ULL << 5U))
#define GPIO_PULL_38_LO() (GPIO_32_THRU_39 &= ~(1ULL << 6U))
#define GPIO_PULL_38_HI() (GPIO_32_THRU_39 |= (1ULL << 6U))
#define GPIO_PULL_39_LO() (GPIO_32_THRU_39 &= ~(1ULL << 7U))
#define GPIO_PULL_39_HI() (GPIO_32_THRU_39 |= (1ULL << 7U))

#define GPIO_GET_0() (GPIO_00_THRU_31 & (1ULL << 0U))
#define GPIO_GET_1() (GPIO_00_THRU_31 & (1ULL << 1U))
#define GPIO_GET_2() (GPIO_00_THRU_31 & (1ULL << 2U))
#define GPIO_GET_3() (GPIO_00_THRU_31 & (1ULL << 3U))
#define GPIO_GET_4() (GPIO_00_THRU_31 & (1ULL << 4U))
#define GPIO_GET_5() (GPIO_00_THRU_31 & (1ULL << 5U))
#define GPIO_GET_6() (GPIO_00_THRU_31 & (1ULL << 6U))
#define GPIO_GET_7() (GPIO_00_THRU_31 & (1ULL << 7U))
#define GPIO_GET_8() (GPIO_00_THRU_31 & (1ULL << 8U))
#define GPIO_GET_9() (GPIO_00_THRU_31 & (1ULL << 9U))
#define GPIO_GET_10() (GPIO_00_THRU_31 & (1ULL << 10U))
#define GPIO_GET_11() (GPIO_00_THRU_31 & (1ULL << 11U))
#define GPIO_GET_12() (GPIO_00_THRU_31 & (1ULL << 12U))
#define GPIO_GET_13() (GPIO_00_THRU_31 & (1ULL << 13U))
#define GPIO_GET_14() (GPIO_00_THRU_31 & (1ULL << 14U))
#define GPIO_GET_15() (GPIO_00_THRU_31 & (1ULL << 15U))
#define GPIO_GET_16() (GPIO_00_THRU_31 & (1ULL << 16U))
#define GPIO_GET_17() (GPIO_00_THRU_31 & (1ULL << 17U))
#define GPIO_GET_18() (GPIO_00_THRU_31 & (1ULL << 18U))
#define GPIO_GET_19() (GPIO_00_THRU_31 & (1ULL << 19U))
#define GPIO_GET_20() (GPIO_00_THRU_31 & (1ULL << 20U))
#define GPIO_GET_21() (GPIO_00_THRU_31 & (1ULL << 21U))
#define GPIO_GET_22() (GPIO_00_THRU_31 & (1ULL << 22U))
#define GPIO_GET_23() (GPIO_00_THRU_31 & (1ULL << 23U))
#define GPIO_GET_24() (GPIO_00_THRU_31 & (1ULL << 24U))
#define GPIO_GET_25() (GPIO_00_THRU_31 & (1ULL << 25U))
#define GPIO_GET_26() (GPIO_00_THRU_31 & (1ULL << 26U))
#define GPIO_GET_27() (GPIO_00_THRU_31 & (1ULL << 27U))
#define GPIO_GET_28() (GPIO_00_THRU_31 & (1ULL << 28U))
#define GPIO_GET_29() (GPIO_00_THRU_31 & (1ULL << 29U))
#define GPIO_GET_30() (GPIO_00_THRU_31 & (1ULL << 30U))
#define GPIO_GET_31() (GPIO_00_THRU_31 & (1ULL << 31U))

#define GPIO_GET_32() (GPIO_32_THRU_39 & (1ULL << 0U))
#define GPIO_GET_33() (GPIO_32_THRU_39 & (1ULL << 1U))
#define GPIO_GET_34() (GPIO_32_THRU_39 & (1ULL << 2U))
#define GPIO_GET_35() (GPIO_32_THRU_39 & (1ULL << 3U))
#define GPIO_GET_36() (GPIO_32_THRU_39 & (1ULL << 4U))
#define GPIO_GET_37() (GPIO_32_THRU_39 & (1ULL << 5U))
#define GPIO_GET_38() (GPIO_32_THRU_39 & (1ULL << 6U))
#define GPIO_GET_39() (GPIO_32_THRU_39 & (1ULL << 7U))

#define GPIO_PULL(PINN, LVL) GPIO_PULL_LITERAL(PINN, LVL)
#define GPIO_PULL_LITERAL(PINN, LVL) GPIO_PULL_##PINN##_##LVL()

#define GPIO_GET(...) GPIO_GET_LITERAL(__VA_ARGS__)
#define GPIO_GET_LITERAL(...) GPIO_GET_##__VA_ARGS__()

#endif // GPIO_H
