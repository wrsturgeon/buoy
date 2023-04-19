#ifndef SPILL_H
#define SPILL_H

// "SPILL": SPI, Low-Level!
// a FUCKING PAIN IN THE ASS to write!
// No third-party libraries here :_)

#include "hardware/gpio.h"
#include "hardware/reg.h"
#include "sane-assert.h"

#include <soc/spi_reg.h>

#include <stdio.h>

#define SPIN 2 // HSPI
#define SPI_MODE 0

#ifndef NDEBUG
static uint8_t SPILL_IS_SET_UP = 0;
static uint8_t SPILL_IS_OPEN = 0;
#endif

// SPI modes: Manual p. 124
#if (SPI_MODE == 0)
#define SPI_CK_IDLE_EDGE_HIGH 0
#define SPI_CK_OUT_EDGE_HIGH 0
#define SPI_MISO_DELAY_MODE_1_HIGH 0
#define SPI_MISO_DELAY_MODE_2_HIGH 1
#elif (SPI_MODE == 1)
#define SPI_CK_IDLE_EDGE_HIGH 0
#define SPI_CK_OUT_EDGE_HIGH 1
#define SPI_MISO_DELAY_MODE_1_HIGH 1
#define SPI_MISO_DELAY_MODE_2_HIGH 0
#elif (SPI_MODE == 2)
#define SPI_CK_IDLE_EDGE_HIGH 1
#define SPI_CK_OUT_EDGE_HIGH 1
#define SPI_MISO_DELAY_MODE_1_HIGH 1
#define SPI_MISO_DELAY_MODE_2_HIGH 0
#elif (SPI_MODE == 3)
#define SPI_CK_IDLE_EDGE_HIGH 1
#define SPI_CK_OUT_EDGE_HIGH 0
#define SPI_MISO_DELAY_MODE_1_HIGH 0
#define SPI_MISO_DELAY_MODE_2_HIGH 1
#else
#error "Unrecognized SPI_MODE"
#endif
__attribute__((always_inline)) inline static void spill_set_mode(void) {
#if SPI_CK_IDLE_EDGE_HIGH
  REG(SPI_PIN_REG(SPIN)) |= (1ULL << SPI_CK_IDLE_EDGE_S);
#else
  REG(SPI_PIN_REG(SPIN)) &= ~(1ULL << SPI_CK_IDLE_EDGE_S);
#endif
#if SPI_CK_OUT_EDGE_HIGH
  REG(SPI_CTRL2_REG(SPIN)) |= (1ULL << SPI_CK_OUT_EDGE_S);
#else
  REG(SPI_CTRL2_REG(SPIN)) &= ~(1ULL << SPI_CK_OUT_EDGE_S);
#endif
#if SPI_MISO_DELAY_MODE_1_HIGH
  REG(SPI_CTRL2_REG(SPIN)) |= (1ULL << SPI_MISO_DELAY_MODE_S);
#else
  REG(SPI_CTRL2_REG(SPIN)) &= ~(1ULL << SPI_MISO_DELAY_MODE_S);
#endif
#if SPI_MISO_DELAY_MODE_2_HIGH
  REG(SPI_CTRL2_REG(SPIN)) |= (1ULL << (SPI_MISO_DELAY_MODE_S + 1));
#else
  REG(SPI_CTRL2_REG(SPIN)) &= ~(1ULL << (SPI_MISO_DELAY_MODE_S + 1));
#endif
}

// Manual p. 128
#define SPI_CLKDIV_PRE_VALUE 8 // max 0x1FFF = 511
#define SPI_CLKCNT_N_VALUE 8   // max 0x003F =  64
__attribute__((always_inline)) inline static void spill_finish_setup(void) {
  // SPI_CLOCK_REG(SPIN)

  // Four-line half-duplex mode: Manual p. 122
  // TODO(wrsturgeon): try three-line, save a pin
  //   - SPI_MISO_DLEN_REG
  //   - SPI_MOSI_DLEN_REG
  REG(SPI_USER_REG(SPIN)) |= SPI_USR_MOSI_M;

  // Manual p. 132
  REG(SPI_CTRL_REG(SPIN)) &= ~(SPI_FREAD_DUAL_M | SPI_FREAD_QUAD_M | SPI_FREAD_DIO_M | SPI_FREAD_QIO_M | SPI_RD_BIT_ORDER_M | SPI_WR_BIT_ORDER_M); // TODO(wrsturgeon): TRY CHANGING BIT ORDERS

  // Finishing SPI_MISO_DELAY_NUM_M, SPI_MOSI_DELAY_MODE_M, & SPI_MOSI_DELAY_NUM_M from mode above, plus Manual p. 134
  REG(SPI_CTRL2_REG(SPIN)) &= ~(SPI_SETUP_TIME_M | SPI_HOLD_TIME_M | SPI_MISO_DELAY_NUM_M | SPI_MOSI_DELAY_MODE_M | SPI_MOSI_DELAY_NUM_M);

  // Manual p. 135
  REG(SPI_CLOCK_REG(SPIN)) &= ~(SPI_CLK_EQU_SYSCLK_M | SPI_CLKDIV_PRE_M | SPI_CLKCNT_H_M | SPI_CLKCNT_L_M | SPI_CLKCNT_N_M);
  REG(SPI_CLOCK_REG(SPIN)) |= (SPI_CLKDIV_PRE_VALUE << SPI_CLKDIV_PRE_S);
  REG(SPI_CLOCK_REG(SPIN)) |= (SPI_CLKCNT_N_VALUE << SPI_CLKCNT_N_S);
  REG(SPI_CLOCK_REG(SPIN)) |= (((SPI_CLKCNT_N_VALUE - 1) >> 1) << SPI_CLKDIV_PRE_S);
  REG(SPI_CLOCK_REG(SPIN)) |= (SPI_CLKCNT_N_VALUE << SPI_CLKCNT_L_S);

  // Manual p. 136-7
  REG(SPI_USER_REG(SPIN)) &= ~(SPI_DOUTDIN_M | SPI_CS_HOLD_M | SPI_CS_SETUP | SPI_CK_I_EDGE_M /* SPI_CK_OUT_EDGE handled above */ | SPI_RD_BYTE_ORDER_M | SPI_WR_BYTE_ORDER_M | SPI_SIO_M | SPI_USR_MOSI_HIGHPART_M | SPI_USR_DUMMY_IDLE_M);
  REG(SPI_USER_REG(SPIN)) |= SPI_USR_MISO_HIGHPART_M;

  // Manual p. 139
  REG(SPI_PIN_REG(SPIN)) |= (SPI_CS1_DIS_M | SPI_CS2_DIS_M /* SPI_CK_IDLE_EDGE handled above */ | SPI_CS_KEEP_ACTIVE_M);
}

// __attribute__((always_inline)) inline static void spill_setup(void) {
//   SANE_ASSERT(!SPILL_IS_SET_UP);
//   spill_set_mode();
//   spill_finish_setup();
//   GPIO_ENABLE_OUTPUT(SPI(CS));
//   GPIO_PULL_HI(SPI(CS));
//   SPILL_IS_SET_UP = 1;
// }

// __attribute__((always_inline)) inline static void spill_open(void) {
//   SANE_ASSERT(SPILL_IS_SET_UP);
//   SANE_ASSERT(!SPILL_IS_OPEN);
//   ASSERT_GPIO_POLL_V(SPI(CS))
//   ();

//   GPIO_PULL_LO(SPI(CS));

//   SPILL_IS_OPEN = 1;
// }

// __attribute__((always_inline)) inline static void spill_close(void) {
//   SANE_ASSERT(SPILL_IS_OPEN);
//   SANE_ASSERT(!GPIO_POLL_V(SPI(CS)));

//   GPIO_PULL_HI(SPI(CS));

//   SPILL_IS_OPEN = 0;
// }

// Manual p. 122 (Four-line half-duplex comm.)
#define SPILL_SAFE_COMMAND(CMD, NBYTE, DELAY, ...)                                                       \
  do {                                                                                                   \
    SANE_ASSERT(SPILL_IS_OPEN);                                                                          \
    REG(SPI_USER_REG(SPIN)) |= SPI_USR_COMMAND_M;                                                        \
    REG(SPI_USER2_REG(SPIN)) &= ~(SPI_USR_COMMAND_BITLEN_M | SPI_USR_COMMAND_VALUE_M);                   \
    REG(SPI_USER2_REG(SPIN)) |= ((7U << SPI_USR_COMMAND_BITLEN_S) | ((CMD) << SPI_USR_COMMAND_VALUE_S)); \
    GPIO_PULL_LO(LCD_DC);                                                                                \
    REG(SPI_CMD_REG(SPIN)) |= SPI_USR_M;                                                                 \
    do { /* busy wait */                                                                                 \
    } while (REG(SPI_CMD_REG(SPIN)) & SPI_USR_M);                                                        \
    GPIO_PULL_HI(LCD_DC);                                                                                \
  } while (0)

typedef enum {
  SPILL_STATE_IDLE,
  SPILL_STATE_PREPARING,
  SPILL_STATE_SENDING_COMMAND,
  SPILL_STATE_SENDING_DATA,
  SPILL_STATE_READING_DATA,
  SPILL_STATE_WRITING_DATA,
  SPILL_STATE_WAITING,
  SPILL_STATE_FINISHED,
} spill_state_t;
__attribute__((always_inline)) inline static spill_state_t spill_get_state(void) { return (REG(SPI_EXT2_REG(SPIN)) & 7U); }
__attribute__((always_inline)) inline static void spill_print_state(void) { printf("SPILL State: %s\r\n", ((char const* const restrict[]){"Idle", "Preparing", "Sending command", "Sending data", "Reading data", "Writing data", "Waiting", "Finished"})[spill_get_state()]); }

// Manual p. 118
__attribute__((always_inline)) inline static void spill_send_8b_data(uint8_t msg) {
  SANE_ASSERT(SPILL_IS_OPEN);

  REG(SPI_W0_REG(SPIN)) = msg;

  // Manual p. 131
  REG(SPI_CMD_REG(SPIN)) |= SPI_USR_M;
  do { // busy wait
    // spill_print_state();
  } while (REG(SPI_CMD_REG(SPIN)) & SPI_USR_M);
}

__attribute__((always_inline)) inline static void spill_send_16b_data(uint16_t msg) {
  spill_send_8b_data(msg >> 8U);
  spill_send_8b_data(msg);
}

#endif // SPILL_H
