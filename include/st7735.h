// This file (st7735.c/.h) is the only one that is NOT MINE;
// it's by J. Ye (provided with UPenn ESE350),
// and his description is below:
//
//     Basic display driver for Adafruit 358 1.8" TFT LCD with ST7735R chip
//     (c) J. Ye, 19 April 2021
//     Version 1.0

#ifndef BUOY_INCLUDE_ST7735_H
#define BUOY_INCLUDE_ST7735_H

#include <avr/io.h>

#define LCD_PORT PORTB
#define LCD_DDR DDRB
#define LCD_DC PORTB0
#define LCD_RST PORTB1
#define LCD_TFT_CS PORTB2
#define LCD_MOSI PORTB3
#define LCD_SCK PORTB5

// LCD_LITE must be connected to pin 6 of Arduino Uno for PWM to change brightness (Otherwise, connect to 5V supply)
#define LCD_LITE_PORT PORTD
#define LCD_LITE_DDR DDRD
#define LCD_LITE PORTD6

#define LCD_WIDTH 160U
#define LCD_HEIGHT 128U
#define LCD_SIZE LCD_WIDTH* LCD_HEIGHT

//! \name Return error codes
//! @{
#define ADAFRUIT358_SPI_NO_ERR 0U                //! No error
#define ADAFRUIT358_SPI_ERR 1U                   //! General or an unknown error
#define ADAFRUIT358_SPI_ERR_RESP_TIMEOUT 2U      //! Timeout during command
#define ADAFRUIT358_SPI_ERR_RESP_BUSY_TIMEOUT 3U //! Timeout on busy signal of R1B response
#define ADAFRUIT358_SPI_ERR_READ_TIMEOUT 4U      //! Timeout during read operation
#define ADAFRUIT358_SPI_ERR_WRITE_TIMEOUT 5U     //! Timeout during write operation
#define ADAFRUIT358_SPI_ERR_RESP_CRC 6U          //! Command CRC error
#define ADAFRUIT358_SPI_ERR_READ_CRC 7U          //! CRC error during read operation
#define ADAFRUIT358_SPI_ERR_WRITE_CRC 8U         //! CRC error during write operation
#define ADAFRUIT358_SPI_ERR_ILLEGAL_COMMAND 9U   //! Command not supported
#define ADAFRUIT358_SPI_ERR_WRITE 10U            //! Error during write operation
#define ADAFRUIT358_SPI_ERR_OUT_OF_RANGE 11U     //! Data access out of range
//! @}

// ST7735 registers
#define ST7735_NOP 0x00U
#define ST7735_SWRESET 0x01U
#define ST7735_RDDID 0x04U
#define ST7735_RDDST 0x09U
#define ST7735_SLPIN 0x10U
#define ST7735_SLPOUT 0x11U
#define ST7735_PTLON 0x12U
#define ST7735_NORON 0x13U
#define ST7735_INVOFF 0x20U
#define ST7735_INVON 0x21U
#define ST7735_DISPOFF 0x28U
#define ST7735_DISPON 0x29U
#define ST7735_CASET 0x2AU
#define ST7735_RASET 0x2BU
#define ST7735_RAMWR 0x2CU
#define ST7735_RAMRD 0x2EU
#define ST7735_PTLAR 0x30U
#define ST7735_COLMOD 0x3AU
#define ST7735_MADCTL 0x36U
#define ST7735_FRMCTR1 0xB1U
#define ST7735_FRMCTR2 0xB2U
#define ST7735_FRMCTR3 0xB3U
#define ST7735_INVCTR 0xB4U
#define ST7735_DISSET5 0xB6U
#define ST7735_PWCTR1 0xC0U
#define ST7735_PWCTR2 0xC1U
#define ST7735_PWCTR3 0xC2U
#define ST7735_PWCTR4 0xC3U
#define ST7735_PWCTR5 0xC4U
#define ST7735_VMCTR1 0xC5U
#define ST7735_RDID1 0xDAU
#define ST7735_RDID2 0xDBU
#define ST7735_RDID3 0xDCU
#define ST7735_RDID4 0xDDU
#define ST7735_PWCTR6 0xFCU
#define ST7735_GMCTRP1 0xE0U
#define ST7735_GMCTRN1 0xE1U

#define MADCTL_MY 0x80U
#define MADCTL_MX 0x40U
#define MADCTL_MV 0x20U
#define MADCTL_ML 0x10U
#define MADCTL_RGB 0x00U
#define MADCTL_BGR 0x08U
#define MADCTL_MH 0x04U

// Macro Functions
#define set(reg, bit) (reg) |= (1U << (bit))
#define clear(reg, bit) (reg) &= ~(1U << (bit))
#define toggle(reg, bit) (reg) ^= (1U << (bit))
#define loop_until_bit_is_set(sfr, bit) \
  do {                                  \
  } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) \
  do {                                    \
  } while (bit_is_set(sfr, bit))

void Delay_ms(unsigned int n);
void lcd_init(void);
void sendCommands(uint8_t const* cmds, uint8_t length);
void LCD_setAddr(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void SPI_ControllerTx(uint8_t data);
void SPI_ControllerTx_stream(uint8_t stream);
void SPI_ControllerTx_16bit(uint16_t data);
void SPI_ControllerTx_16bit_stream(uint16_t data);
void LCD_brightness(uint8_t intensity);
void LCD_rotate(uint8_t r);

#endif // BUOY_INCLUDE_ST7735_H
