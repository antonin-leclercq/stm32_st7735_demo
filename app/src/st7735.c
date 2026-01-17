/*
 * st7735.c
 *
 *  Created on: 8 févr. 2024
 *      Author: anton
 */


#include "st7735.h"

__IO uint8_t flag__dma1_channel3_done;

void ST7735_Init(void) {

	// Using SPI1, 8 bits / bi-directionnal interface
	//
	// pins used:
	//    - PA5 (D13) as SPI_SCK (AF5)
	//    - PA7 (D11) as SPI_MOSI (AF5)
	//      In this case, PA7 is SDA and is a bi-di line so we make it open-drain
	//	  - PA4 (A2)  as SPI_CS (software controlled)
	//
	//    - PA9 (D8) : DC (data / command select)
	//    - PA10 (D2) : RST (chip reset)
	//      RST low => chip initialization
	//      RST high => normal operation
	//    - PA11 : BLK (back light control)
	//
	// F(PCLK) = F(PCLK2) = 64MHz
	// (Debug/Troubleshooting purposes) Baud rate is 250kHz => BR = /256
	//
	// Default pixel color format : 18bits / pixel (6/6/6)
	//
	// Using DMA1 Channel 3 (SPI_TX) to unload CPU for frame transmission
	// Memory to peripheral => frame_buffer to SPI1->DR
	// Memory size and peripheral size are 8 bits (default)
	// Memory increment enabled, peripheral increment disabled
	// Circular mode disabled (since frame_buffer is constant / not updated)

	// Enable GPIOA clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// Configure pins (High-speed, AF5)
	GPIOA->MODER &= ~(GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE7_Msk);
	GPIOA->MODER |= (0x02 << GPIO_MODER_MODE5_Pos) | (0x02 << GPIO_MODER_MODE7_Pos);
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED5_Msk | GPIO_OSPEEDR_OSPEED7_Msk);
	GPIOA->OSPEEDR |= (0x02 << GPIO_OSPEEDR_OSPEED5_Pos) | (0x02 << GPIO_OSPEEDR_OSPEED7_Pos);
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL5_Msk | GPIO_AFRL_AFSEL7_Msk);
	GPIOA->AFR[0] |= (0x05 << GPIO_AFRL_AFSEL5_Pos) | (0x05 << GPIO_AFRL_AFSEL7_Pos);

	// PA4 (CS) as high-speed output GPIO
	GPIOA->MODER &= ~GPIO_MODER_MODE4_Msk;
	GPIOA->MODER |= (0x01 << GPIO_MODER_MODE4_Pos);
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED4_Msk;
	GPIOA->OSPEEDR |= (0x02 << GPIO_OSPEEDR_OSPEED4_Pos);

	// CS active low
	GPIOA->ODR |= GPIO_ODR_OD4;

	// Other ST7735-related GPIOs (BKL, DC, RST)
	GPIOA->MODER &= ~(GPIO_MODER_MODE9_Msk | GPIO_MODER_MODE10_Msk | GPIO_MODER_MODE11_Msk);
	GPIOA->MODER |= (0x01 << GPIO_MODER_MODE9_Pos) | (0x01 << GPIO_MODER_MODE10_Pos) | (0x01 << GPIO_MODER_MODE11_Pos);
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED9_Msk | GPIO_OSPEEDR_OSPEED10_Msk | GPIO_OSPEEDR_OSPEED11_Msk);
	GPIOA->OSPEEDR |= (0x02 << GPIO_OSPEEDR_OSPEED9_Pos) | (0x02 << GPIO_OSPEEDR_OSPEED10_Pos) | (0x02 << GPIO_OSPEEDR_OSPEED11_Pos);

	// RST is high by default
	GPIOA->ODR |= GPIO_ODR_OD10;

	//////////////////////////////////////////////// end of GPIO configuration, begin DMA initialization

	// Enable DMA1
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	// Reset DMA1 channel 3 configuration
	DMA1_Channel3->CCR = 0x00000000;
	DMA1_Channel3->CNDTR = 0x00000000;

	// Set priority to medium
	DMA1_Channel3->CCR |= (0x01 << DMA_CCR_PL_Pos);

	// Enable memory increment
	DMA1_Channel3->CCR |= DMA_CCR_MINC;

	// Set direction memory => peripheral
	DMA1_Channel3->CCR |= DMA_CCR_DIR;

	// Enable Transfer complete interrupt
	DMA1_Channel3->CCR |= DMA_CCR_TCIE;

	// Set number of data to transfer (number of pixels * 3(RGB))
	DMA1_Channel3->CNDTR = (uint32_t) PIXEL_WIDTH * PIXEL_HEIGHT * 3;

	// Set peripheral address
	DMA1_Channel3->CPAR = (uint32_t) &SPI1->DR;

	// Set memory address
	DMA1_Channel3->CMAR = (uint32_t) frame_buffer;

	// Map DMA to SPI1_TX
	DMA1_CSELR->CSELR &= ~DMA_CSELR_C3S;
	DMA1_CSELR->CSELR |= (0x01 << DMA_CSELR_C3S_Pos);


	//////////////////////////////////////////////// end of DMA configuration, begin SPI initialization

	// Enable SPI1 clock
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	// Reset configuration
	SPI1->CR1 = 0x0000;
	SPI1->CR2 = 0x0700;

	// Select Bi-directionnal mode
	SPI1->CR1 |= SPI_CR1_BIDIMODE;

	// MCU is master
	SPI1->CR1 |= SPI_CR1_MSTR;

	// Enable software slave management
	SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;

	// Set Baud rate to 250kHz
	SPI1->CR1 |= (0x07 << SPI_CR1_BR_Pos);

	// Set SPI1 FIFO RX threshold to 8 bit
	SPI1->CR2 |= SPI_CR2_FRXTH;

	// Transmit only mode first
	SPI1->CR1 |= SPI_CR1_BIDIOE;

	// Enable SPI1
	SPI1->CR1 |= SPI_CR1_SPE;

////////////////////////////////////////////////// end of SPI configuration, begin LCD initialization

	// Reset ST7735
	// Hardware reset
	ST7735_HWReset();
	TIM_Delay_Milli(130);

	// Software reset
	ST7735_SendCommand(SWRESET);

	// Must wait at least 120ms after SW reset
	TIM_Delay_Milli(130);

	// Sleep out
	ST7735_SendCommand(SLPOUT);

	// Must wait at least 120ms after SLPOUT
	TIM_Delay_Milli(130);

	// Set column and row address sets to full screen
	ST7735_SetColumnAddress(0, PIXEL_WIDTH-1);
	ST7735_SetRowAddress(0, PIXEL_HEIGHT-1);

	// Display ON
	ST7735_SendCommand(DISPON);
}

void ST7735_NVIC_Init(void) {
	// Priority is set to 1, (high priority)
	NVIC_SetPriority(DMA1_Channel3_IRQn, 1);
	NVIC_EnableIRQ(DMA1_Channel3_IRQn);
}

void ST7735_WriteByte(const uint8_t byte) {
	// Transmit only mode
	SPI1->CR1 |= SPI_CR1_BIDIOE;

	// wait for TX buffer to empty
	while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);

	// write byte
	*(__IO uint8_t*)&SPI1->DR = byte;

	// wait while SPI is busy
	while((SPI1->SR & SPI_SR_BSY) != 0);
}

void ST7735_WriteWord(const uint16_t word) {
	// Transmit only mode
	SPI1->CR1 |= SPI_CR1_BIDIOE;

	// wait for TX buffer to empty
	while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);

	// write byte
	SPI1->DR = word;

	// wait while SPI is busy
	while((SPI1->SR & SPI_SR_BSY) != 0);
}

void ST7735_ReadBytes(const uint8_t address, uint8_t* bytes, const uint8_t n) {
	// When reading we must disable SPI then re-enable it in order to generate clock signal

	//////////////////////////////////////////// Sending the address of the register we want to read
	//											 Not using the existing ST7735_SendCommand function because we want CS to remain low
	// Command => DC Low
	GPIOA->ODR &= ~GPIO_ODR_OD9;

	// Set CS low
	GPIOA->ODR &= ~GPIO_ODR_OD4;

	if (n >= 2) {
		// 9 bit data to make host output a dummy clock cycle required when reading >= 2 bytes
		// NOTE: Make sure to write to SPI DS the right way because if the register is cleared (=0x0) it is automatically set back to 0x07
		SPI1->CR2 |= (0x08 << SPI_CR2_DS_Pos);
		SPI1->CR2 &= ~(0x07 << SPI_CR2_DS_Pos);
		ST7735_WriteWord(address << 1);
	}
	else {
		ST7735_WriteByte(address);
	}

	/////////////////////////////////////////////////// Start reading

	// Disable SPI
	SPI1->CR1 &= ~SPI_CR1_SPE;

	if (n >= 2)	{
		// Go back to 8 bit data size
		// NOTE: Make sure to write to SPI DS the right way because if the register is cleared (=0x0) it is automatically set back to 0x07
		SPI1->CR2 |= (0x07 << SPI_CR2_DS_Pos);
		SPI1->CR2 &= ~(0x08 << SPI_CR2_DS_Pos);

		// Wait another 4µs, not sure why but it works @ Baud rate = 250kHz
		TIM_Delay_Micro(4);
	}

	// Receive only mode then
	SPI1->CR1 &= ~SPI_CR1_BIDIOE;

	// DC high when reading
	GPIOA->ODR |= GPIO_ODR_OD9;

	// Enable SPI back
	SPI1->CR1 |= SPI_CR1_SPE;

	for (uint8_t i = 0; i < n; ++i) {
		// wait for RX buffer to not be empty
		while((SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE);

		// receive data
		*(bytes + i) = *(__IO uint8_t*)&SPI1->DR;
	}

	// Set CS high
	GPIOA->ODR |= GPIO_ODR_OD4;

	// Back to Transmit only mode
	SPI1->CR1 |= SPI_CR1_BIDIOE;
}

void ST7735_WriteBytes(const uint8_t address, const uint8_t* bytes, const uint32_t n) {
	// Send address we want to write to
	ST7735_SendCommand(address);

	// DC has to be high (data)
	GPIOA->ODR |= GPIO_ODR_OD9;

	// Set CS low
	GPIOA->ODR &= ~GPIO_ODR_OD4;

	// Loop through bytes
	for (uint32_t i = 0; i < n; ++i) {

		// wait for TX buffer to empty
		while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);

		// write byte
		*(__IO uint8_t*)&SPI1->DR = *(bytes + i);
	}

	// wait while SPI is busy
	while((SPI1->SR & SPI_SR_BSY) != 0);

	// Set CS high
	GPIOA->ODR |= GPIO_ODR_OD4;
}

/////////////////////////////////////////////// Function to fill the LCD RAM
void ST7735_MemoryWrite(const uint8_t* buffer, const uint8_t frame_x_size, const uint8_t frame_y_size,
		const uint8_t x_start, const uint8_t y_start) {
	// Writing to the LCD frame memory with RGB format 6-6-6
	// Note that for other formats like 4-4-4 or 5-6-5, the data transmission is different

	// Calculate end point
	const uint8_t x_end = x_start + frame_x_size -1;
	const uint8_t y_end = y_start + frame_y_size -1;

	// Set memory zone to write to
	ST7735_SetColumnAddress(x_start, x_end);
	ST7735_SetRowAddress(y_start, y_end);

	// Write to controller memory
	ST7735_WriteBytes(RAMWR, buffer, frame_x_size*frame_y_size*3);
}

void ST7735_MemoryWriteDMA(void) {
	// Writing to the LCD frame memory with RGB format 6-6-6
	// Note that for other formats like 4-4-4 or 5-6-5, the data transmission is different

	// Write to RAM
	ST7735_SendCommand(RAMWR);

	// DC has to be high (data)
	GPIOA->ODR |= GPIO_ODR_OD9;

	// Set CS low
	GPIOA->ODR &= ~GPIO_ODR_OD4;

	// Enable DMA1_Channel3
	DMA1_Channel3->CCR |= DMA_CCR_EN;

	// Enable TX DMA requests
	SPI1->CR2 |= SPI_CR2_TXDMAEN;

	flag__dma1_channel3_done = 0;

	// DMA is now handling the data transfer from our frame_buffer to the SPI peripheral

	// Disabling DMA after transfer complete and setting CS back to high is done in the ISR (find code in stm32l4xx_it.c)
}


void ST7735_SendData(const uint8_t data) {
	// Data => DC High
	GPIOA->ODR |= GPIO_ODR_OD9;

	// Set CS low
	GPIOA->ODR &= ~GPIO_ODR_OD4;

	// Send data
	ST7735_WriteByte(data);

	// Set CS high
	GPIOA->ODR |= GPIO_ODR_OD4;
}

void ST7735_SendCommand(const uint8_t command) {
	// Command => DC Low
	GPIOA->ODR &= ~GPIO_ODR_OD9;

	// Set CS low
	GPIOA->ODR &= ~GPIO_ODR_OD4;

	// Send command
	ST7735_WriteByte(command);

	// Set CS high
	GPIOA->ODR |= GPIO_ODR_OD4;
}

void ST7735_SetBacklight(const enum BL_STATE state) {
	if (state == BL_ON) GPIOA->ODR |= GPIO_ODR_OD11;
	else GPIOA->ODR &= ~GPIO_ODR_OD11;
}

void ST7735_HWReset(void) {
	// RST low
	GPIOA->ODR &= ~GPIO_ODR_OD10;

	// Wait a bit
	TIM_Delay_Milli(10);

	// RST high
	GPIOA->ODR |= GPIO_ODR_OD10;
}

void ST7735_ReadID(uint8_t* id_buffer, const enum WHICH_ID id) {
	switch (id) {
	case ALL_IDs:
		// Reading more than a byte requires a dummy clock cycle put by the host after the command / register address
		ST7735_ReadBytes(RDDID, id_buffer, 3);
		break;
	case ID1:
		ST7735_ReadBytes(RDID1, id_buffer, 1);
		break;
	case ID2:
		ST7735_ReadBytes(RDID2, id_buffer, 1);
		break;
	case ID3:
		ST7735_ReadBytes(RDID3, id_buffer, 1);
		break;
	default:
		break;
	}
}

void ST7735_SetColumnAddress(const uint8_t xs, const uint8_t xe) {
	if (xe < xs || xe > PIXEL_WIDTH-1) return;

	const uint8_t bytes[] = {
			0, xs, 0, xe
	};

	ST7735_WriteBytes(CASET, bytes, 4);
}

void ST7735_SetRowAddress(const uint8_t ys, const uint8_t ye) {
	if (ye < ys || ye > PIXEL_HEIGHT-1) return;

	const uint8_t bytes[] = {
			0, ys, 0, ye
	};

	ST7735_WriteBytes(RASET, bytes, 4);
}

void ST7735_SetMirror(const uint32_t x_mirror, const uint32_t y_mirror)
{
	// Read current MADCTL configuration
	uint8_t madtcl = 0;
	ST7735_ReadBytes(RDDMADTCL, &madtcl, 1);

	// Update with parameters
	madtcl &= ~0b11000000;
	madtcl |= ((x_mirror & 0x01) << 6) | ((y_mirror & 0x01) << 7);

	// Send back to controller
	ST7735_WriteBytes(MADTCL, &madtcl, 1);
}

void ST7735_DrawRectangle(const uint8_t x_start, const uint8_t y_start, const uint8_t x_end, const uint8_t y_end, const uint32_t color)
{
	// Color format:
	// For 6-6-6 color format:
	// Use first 18LSBs, upper 6 bits are red, lower 6 bits are blue, send blue component first

	ST7735_SetColumnAddress(x_start, x_end);
	ST7735_SetRowAddress(y_start, y_end);

	const uint32_t size = (x_end - x_start + 1) * (y_end - y_start + 1);

	// Extract RGB 6-6-6 colors and left shift twice each component
	const uint8_t bytes[] = { ((color & 0x3F000) >> 12) << 2,
							((color & 0xFC0) >> 6) << 2,
							(color & 0x3F) << 2 };

	// Send address we want to write to
	ST7735_SendCommand(RAMWR);

	// DC has to be high (data)
	GPIOA->ODR |= GPIO_ODR_OD9;

	// Set CS low
	GPIOA->ODR &= ~GPIO_ODR_OD4;

	// Loop through bytes
	for (uint32_t i = 0; i < size; ++i) {

		// wait for TX buffer to empty
		while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);

		// write red byte
		*(__IO uint8_t*)&SPI1->DR = *(bytes);

		// wait for TX buffer to empty
		while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);

		// write green byte
		*(__IO uint8_t*)&SPI1->DR = *(bytes + 1);

		// wait for TX buffer to empty
		while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);

		// write blue byte
		*(__IO uint8_t*)&SPI1->DR = *(bytes + 2);
	}

	// wait while SPI is busy
	while((SPI1->SR & SPI_SR_BSY) != 0);

	// Set CS high
	GPIOA->ODR |= GPIO_ODR_OD4;
}
