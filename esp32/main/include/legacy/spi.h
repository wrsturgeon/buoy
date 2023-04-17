#ifndef SPI_H
#define SPI_H

// See page 120 (7.3: GP-SPI)
// https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf

#include "hardware/gpio.h"
#include "pins.h"
#include "sane-assert.h"

#include <driver/spi_master.h>
#include <soc/spi_reg.h>

#include <stdint.h>

static spi_device_handle_t FUCK_IT_GLOBAL_SPI_HANDLE;

#define NO_DMA 0 // for clarity of intention

void spi_pre_transfer_callback(spi_transaction_t* t) {
  if (t->user) {
    GPIO_PULL_HI(LCD_DC);
  } else {
    GPIO_PULL_LO(LCD_DC);
  }
}

#ifndef NDEBUG
static uint8_t SPI_INITIALIZED = 0;
static uint8_t SPI_COMM_OPEN = 0;
#endif

static spi_bus_config_t const SPI_BUS_CONFIG = {
    .mosi_io_num = SPI(MOSI),
    .miso_io_num = SPI(MISO),
    .sclk_io_num = SPI(CLK),
    .data2_io_num = -1,
    .data3_io_num = -1,
    .data4_io_num = -1,
    .data5_io_num = -1,
    .data6_io_num = -1,
    .data7_io_num = -1,
    .max_transfer_sz = 0,
    .flags = (SPICOMMON_BUSFLAG_MASTER),
    .isr_cpu_id = 0,
    .intr_flags = 0,
};

static spi_device_interface_config_t const SPI_DEVICE_CONFIG = {
    .command_bits = 8,
    .address_bits = 0, // TODO: verify
    .dummy_bits = 0,
    .mode = 0,
    .clock_source = SPI_CLK_SRC_DEFAULT,
    .duty_cycle_pos = 128,
    .cs_ena_pretrans = 0,
    .cs_ena_posttrans = 0,
    .clock_speed_hz = 10000000 /* 10MHz */,
    .input_delay_ns = 0,
    .spics_io_num = SPI(CS),
    .flags = 0,
    .queue_size = 7,
    .pre_cb = spi_pre_transfer_callback,
    .post_cb = 0,
};

__attribute__((always_inline)) inline static void spi_init(void) {
#ifndef NDEBUG
  SANE_ASSERT(!SPI_INITIALIZED);
  SPI_INITIALIZED = 1;
#endif

  ESP_ERROR_CHECK(spi_bus_initialize(SPI(BUS), &SPI_BUS_CONFIG, NO_DMA));

  ESP_ERROR_CHECK(spi_bus_add_device(SPI(BUS), &SPI_DEVICE_CONFIG, &FUCK_IT_GLOBAL_SPI_HANDLE));

  ESP_ERROR_CHECK(spi_device_acquire_bus(FUCK_IT_GLOBAL_SPI_HANDLE, portMAX_DELAY));

  // TODO: REENABLE vvv
  // GPIO_ENABLE_OUTPUT(SPI(CS));

  // Manual p. 128
  // REG_WRITE(SPI_CTRL_REG(SPI_HOST), ...);
  // REG_WRITE(SPI_CTRL2_REG(SPI_HOST), ...);
  // REG_WRITE(SPI_CLOCK_REG(SPI_HOST), ...);
  // REG_WRITE(SPI_PIN_REG(SPI_HOST), ...);
  // REG_WRITE(SPI_CMD_REG(SPI_HOST), ...);
  // REG_WRITE(SPI_ADDR_REG(SPI_HOST), ...);
  // REG_WRITE(SPI_USER_REG(SPI_HOST), ...);
  // REG_WRITE(SPI_USER1_REG(SPI_HOST), ...);
  // REG_WRITE(SPI_USER2_REG(SPI_HOST), ...);
  // REG_WRITE(SPI_MOSI_DLEN_REG(SPI_HOST), ...);
}

__attribute__((always_inline)) inline static void spi_open_comm(void) {
#ifndef NDEBUG
  SPI_COMM_OPEN = 1;
#endif
  // TODO: REENABLE vvv
  // GPIO_PULL_LO(SPI(CS));
}

__attribute__((always_inline)) inline static void spi_close_comm(void) {
#ifndef NDEBUG
  SPI_COMM_OPEN = 0;
#endif
  // TODO: REENABLE vvv
  // GPIO_PULL_HI(SPI(CS));
}

#include </Users/willsturgeon/esp/esp-idf/components/driver/spi/gpspi/spi_master.c>

__attribute__((always_inline)) inline static void spi_trust_send_8b(uint8_t const data) {
  // Arduino implementation:
  //    SPDR = stream;
  //    while (!(SPSR & (1U << SPIF)));
  SANE_ASSERT(SPI_COMM_OPEN);

  spi_transaction_t trans = {
      .length = 8, // bits
      .tx_buffer = &data,
      .user = 0,
      .flags = SPI_TRANS_CS_KEEP_ACTIVE,
  };

  ESP_ERROR_CHECK(check_trans_valid(FUCK_IT_GLOBAL_SPI_HANDLE, &trans));
  if ((!(!spi_bus_device_is_polling(FUCK_IT_GLOBAL_SPI_HANDLE)))) {
    if ((CONFIG_LOG_MAXIMUM_LEVEL >= (ESP_LOG_ERROR) && esp_log_default_level >= (ESP_LOG_ERROR))) { esp_rom_printf(
        "E (%u) %s: %s(%d): Cannot send polling transaction while the previous polling transaction is not terminated.\n",
        esp_log_timestamp(),
        SPI_TAG,
        __FUNCTION__,
        124); }
    ESP_ERROR_CHECK(0x103);
  }

  /* If device_acquiring_lock is set to FUCK_IT_GLOBAL_SPI_HANDLE, it means that the user has already
   * acquired the bus thanks to the function `spi_device_acquire_bus()`.
   * In that case, we don't need to take the lock again. */
  spi_host_t* host = FUCK_IT_GLOBAL_SPI_HANDLE->host;
  if (host->device_acquiring_lock != FUCK_IT_GLOBAL_SPI_HANDLE) {
    /* The user cannot ask for the CS to keep active has the bus is not locked/acquired. */
    if ((trans.flags & SPI_TRANS_CS_KEEP_ACTIVE) != 0) {
      ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    } else {
      ESP_ERROR_CHECK(spi_bus_lock_acquire_start(FUCK_IT_GLOBAL_SPI_HANDLE->dev_lock, portMAX_DELAY));
    }
  } else {
    ESP_ERROR_CHECK(spi_bus_lock_wait_bg_done(FUCK_IT_GLOBAL_SPI_HANDLE->dev_lock, portMAX_DELAY));
  }

  ESP_ERROR_CHECK(setup_priv_desc(&trans, &host->cur_trans_buf, (host->bus_attr->dma_enabled)));

  // Polling, no interrupt is used.
  host->polling = true;

  ESP_LOGV(SPI_TAG, "polling trans");
  spi_new_trans(FUCK_IT_GLOBAL_SPI_HANDLE, &host->cur_trans_buf);

  ESP_ERROR_CHECK(spi_device_polling_end(FUCK_IT_GLOBAL_SPI_HANDLE, portMAX_DELAY));

  //%%%%%%%%%%%%%%%% ~/esp/esp-idf/components/driver/spi/gpspi/spi_master.c:1027 to end:
  // Summarized:
  //   - Validate input
  //   - Make sure the bus isn't in use
  //   - Acquire a lock with `spi_bus_lock_wait_bg_done` or `spi_bus_lock_acquire_start`
  //   - Setup...something (`priv_desc`?)
  //   - Set polling to true (this must be used later somewhere)
  //   - Call `spi_new_trans` (I'm guessing this is where the magic happens)
  //
  // Source code:
  // esp_err_t SPI_MASTER_ISR_ATTR spi_device_polling_start(spi_device_handle_t handle, spi_transaction_t * trans_desc, TickType_t ticks_to_wait) {
  //   esp_err_t ret;
  //   SPI_CHECK(ticks_to_wait == portMAX_DELAY, "currently timeout is not available for polling transactions", ESP_ERR_INVALID_ARG);
  //   ret = check_trans_valid(handle, trans_desc);
  //   if (ret != ESP_OK) return ret;
  //   SPI_CHECK(!spi_bus_device_is_polling(handle), "Cannot send polling transaction while the previous polling transaction is not terminated.", ESP_ERR_INVALID_STATE);

  //   /* If device_acquiring_lock is set to handle, it means that the user has already
  //    * acquired the bus thanks to the function `spi_device_acquire_bus()`.
  //    * In that case, we don't need to take the lock again. */
  //   spi_host_t* host = handle->host;
  //   if (host->device_acquiring_lock != handle) {
  //     /* The user cannot ask for the CS to keep active has the bus is not locked/acquired. */
  //     if ((trans_desc->flags & SPI_TRANS_CS_KEEP_ACTIVE) != 0) {
  //       ret = ESP_ERR_INVALID_ARG;
  //     } else {
  //       ret = spi_bus_lock_acquire_start(handle->dev_lock, ticks_to_wait);
  //     }
  //   } else {
  //     ret = spi_bus_lock_wait_bg_done(handle->dev_lock, ticks_to_wait);
  //   }
  //   if (ret != ESP_OK) return ret;

  //   ret = setup_priv_desc(trans_desc, &host->cur_trans_buf, (host->bus_attr->dma_enabled));
  //   if (ret != ESP_OK) return ret;

  //   // Polling, no interrupt is used.
  //   host->polling = true;

  //   ESP_LOGV(SPI_TAG, "polling trans");
  //   spi_new_trans(handle, &host->cur_trans_buf);

  //   return ESP_OK;
  // }

  // esp_err_t SPI_MASTER_ISR_ATTR spi_device_polling_end(spi_device_handle_t handle, TickType_t ticks_to_wait) {
  //   SPI_CHECK(handle != NULL, "invalid dev handle", ESP_ERR_INVALID_ARG);
  //   spi_host_t* host = handle->host;

  //   SANE_ASSERT(host->cur_cs == handle->id);
  //   SANE_ASSERTSSERT(handle == get_acquiring_dev(host));

  //   TickType_t start = xTaskGetTickCount();
  //   while (!spi_hal_usr_is_done(&host->hal)) {
  //     TickType_t end = xTaskGetTickCount();
  //     if (end - start > ticks_to_wait) {
  //       return ESP_ERR_TIMEOUT;
  //     }
  //   }

  //   ESP_LOGV(SPI_TAG, "polling trans done");
  //   // deal with the in-flight transaction
  //   spi_post_trans(host);
  //   // release temporary buffers
  //   uninstall_priv_desc(&host->cur_trans_buf);

  //   host->polling = false;
  //   /* Once again here, if device_acquiring_lock is set to `handle`, it means that the user has already
  //    * acquired the bus thanks to the function `spi_device_acquire_bus()`.
  //    * In that case, the lock must not be released now because . */
  //   if (host->device_acquiring_lock != handle) {
  //     SANE_ASSERT(host->device_acquiring_lock == NULL);
  //     spi_bus_lock_acquire_end(handle->dev_lock);
  //   }

  //   return ESP_OK;
  // }

  // esp_err_t SPI_MASTER_ISR_ATTR spi_device_polling_transmit(spi_device_handle_t handle, spi_transaction_t * trans_desc) {
  //   esp_err_t ret;
  //   ret = spi_device_polling_start(handle, trans_desc, portMAX_DELAY);
  //   if (ret != ESP_OK) return ret;

  //   return spi_device_polling_end(handle, portMAX_DELAY);
  // }
}

__attribute__((always_inline)) inline static void spi_trust_send_16b(uint16_t const data) {
  spi_trust_send_8b(data >> 8U); // more significant/leftmost bits
  spi_trust_send_8b(data & 255); // less significant/rghtmost bits
}

#endif // SPI_H
