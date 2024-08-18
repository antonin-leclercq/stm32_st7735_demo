/*
 * st7735.h
 *
 *  Created on: 8 févr. 2024
 *      Author: anton
 */

#ifndef APP_INC_ST7735_H_
#define APP_INC_ST7735_H_

#include "delay.h"

#define PIXEL_WIDTH 128
#define PIXEL_HEIGHT 160

// System function commands
#define NOP			0x00 // no operation
#define SWRESET		0x01 // software reset
#define RDDID		0x04 // read IDs
#define RDDST		0x09 // read status
#define RDDPM		0x0A // read power mode
#define RDDMADTCL	0x0B // read MADTCL
#define RDDCOLMOD	0x0C // read pixel format
#define RDDIM		0x0D // read image mode
#define RDDSM		0x0E // read signal mode
#define RDDSRD		0x0F // read self-diagnostic result
#define SLPIN		0x10 // sleep in & booster off
#define SLPOUT		0x11 // sleep out & booster on
#define PTLON		0x12 // partial mode on
#define NORON		0x13 // partial mode off (normal)
#define INVOFF		0x20 // display inversion off (normal)
#define INVON		0x21 // display inversion on
#define GAMSET		0x26 // gamma curve select
#define DISPOFF		0x28 // display off
#define DISPON		0x29 // display on
#define CASET		0x2A // column address set
#define RASET		0x2B // row address set
#define RAMWR		0x2C // memory write
#define RGBSET		0x2D // LUT for 4k, 65k, 262k color
#define RAMRD		0x2E // memory read
#define PTLAR		0x30 // partial start/end address set
#define SCRLAR		0x33 // scroll area set
#define TEOFF		0x34 // tearing effect line off
#define TEON		0x35 // tearing effect line on
#define MADTCL		0x36 // memory data access control
#define VSCSAD		0x37 // scroll start address of RAM
#define IDMOFF		0x38 // idle mode off
#define IDMON		0x39 // idle mode on
#define COLMOD		0x3A // interface pixel format
#define RDID1		0xDA // read ID1
#define RDID2		0xDB // read ID2
#define RDID3		0xDC // read ID3

// Panel function commands
#define FRMCTR1		0xB1 // in normal mode
#define FRMCTR2		0xB2 // in idle mode
#define FRMCTR3		0xB3 // in partial mode + full colors
#define INVCTR		0xB4 // display inversion control
#define PWCTR1		0xC0 // power control setting
#define PWCTR2		0xC1 // power control setting
#define PWCTR3		0xC2 // in normal mode
#define PWCTR4		0xC3 // in idle mode
#define PWCTR5		0xC4 // in partial mode + full colors
#define VMCTR1		0xC5 // VCOM control 1
#define VMOFCTR		0xC7 // set VCOM offset control
#define WRID2		0xD1 // set LCM (ID2) version code
#define WRID3		0xD2 // set customer project (ID3) code
#define NVCTR1		0xD9 // NVM control status
#define NVCTR2		0xDE // NVM read command
#define NVCTR3		0xDF // NVM write command
#define GAMCTRP1	0xE0 // set Gamma adjustment (+ polarity)
#define GAMCTRN1	0xE1 // set Gamma adjustment (- polarity)
#define GCV			0xFC // Gate clock variable


enum STATE {
	OFF,
	ON,
};

enum WHICH_ID {
	ALL_IDs,
	ID1,
	ID2,
	ID3,
};

// Found in st7735_frame.c
// Frame buffer for RGB 6-6-6 format
extern const uint8_t frame_buffer[PIXEL_WIDTH * PIXEL_HEIGHT * 3];

void ST7735_Init(void);
void ST7735_NVIC_Init(void);

void ST7735_WriteByte(const uint8_t byte);
void ST7735_WriteWord(const uint16_t word);

// TODO : fix this function
void ST7735_ReadBytes(const uint8_t address, uint8_t* bytes, const uint8_t n);
void ST7735_WriteBytes(const uint8_t address, const uint8_t* bytes, const uint32_t n);

void ST7735_MemoryWrite(void);
void ST7735_MemoryWriteDMA(void);

void ST7735_SendData(const uint8_t data);

void ST7735_SendCommand(const uint8_t command);

// TODO : fix this function
void ST7735_ReadID(uint8_t* id_buffer, const enum WHICH_ID id);

void ST7735_HWReset(void);

void ST7735_SetBacklight(const enum STATE state);

void ST7735_ColumnAddressSet(const uint8_t xs, const uint8_t xe);
void ST7735_RowAddressSet(const uint8_t ys, const uint8_t ye);

#endif /* APP_INC_ST7735_H_ */
