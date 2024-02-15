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

// ST7735S commands
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

void ST7735_Init(void);

// TODO : fix this function
void ST7735_ReadBytes(const uint8_t address, uint8_t* bytes, const uint8_t n);

void ST7735_WriteByte(const uint8_t byte);

void ST7735_MemoryWrite(void);

void ST7735_SendData(const uint8_t data);

void ST7735_SendCommand(const uint8_t command);

// TODO : fix this function
void ST7735_ReadID(uint8_t* id_buffer, const enum WHICH_ID id);

void ST7735_HWReset(void);

void ST7735_SetBacklight(const enum STATE state);

#endif /* APP_INC_ST7735_H_ */
