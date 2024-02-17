/*
 * st7735.c
 *
 *  Created on: 8 févr. 2024
 *      Author: anton
 */


#include "st7735.h"

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

////////////////////////////////////////////////// end of STM32 configuration, begin LCD initialization

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

	// Display ON
	ST7735_SendCommand(DISPON);
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

// TODO : fix this function
void ST7735_ReadBytes(const uint8_t address, uint8_t* bytes, const uint8_t n) {
	// When reading we must disable SPI then re-enable it in order to generate clock signal

	//////////////////////////////////////////// Sending the address of the register we want to read
	//											 Not using the existing ST7735_SendCommand function because we want CS to remain low
	// Command => DC Low
	GPIOA->ODR &= ~GPIO_ODR_OD9;

	// Set CS low
	GPIOA->ODR &= ~GPIO_ODR_OD4;

	ST7735_WriteByte(address);

	/////////////////////////////////////////////////// Start reading

	// Disable SPI
	SPI1->CR1 &= ~SPI_CR1_SPE;

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

/////////////////////////////////////////////// Function to fill the LCD RAM
void ST7735_MemoryWrite(void) {
	// Writing to the LCD frame memory with RGB format 6-6-6
	// Note that for other formats like 4-4-4 or 5-6-5, the data transmission is different

	// Write to RAM
	ST7735_SendCommand(RAMWR);

	// DC has to be high (data)
	GPIOA->ODR |= GPIO_ODR_OD9;

	// Set CS low
	GPIOA->ODR &= ~GPIO_ODR_OD4;

	// Loop through all pixels
	for (uint32_t i = 0; i < PIXEL_WIDTH*PIXEL_HEIGHT*3; ++i) {

		// wait for TX buffer to empty
		while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);

		// write byte
		*(__IO uint8_t*)&SPI1->DR = *(frame_buffer + i);
	}

	// wait while SPI is busy
	while((SPI1->SR & SPI_SR_BSY) != 0);

	// Set CS high
	GPIOA->ODR |= GPIO_ODR_OD4;
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

void ST7735_SetBacklight(const enum STATE state) {
	if (state == ON) GPIOA->ODR |= GPIO_ODR_OD11;
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

// TODO : fix this function
void ST7735_ReadID(uint8_t* id_buffer, const enum WHICH_ID id) {
	switch (id) {
	case ALL_IDs:
		// TODO: reading all IDs at once doesn't work, maybe due to a required "dummy clock"
		ST7735_ReadBytes(RDDID, id_buffer, 4);
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
