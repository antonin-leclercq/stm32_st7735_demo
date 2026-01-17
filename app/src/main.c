/*
 * main.c
 *
 *  Created on: 6 févr. 2024
 *      Author: anton
 */


#include "main.h"

static void System_ClockInit(void);

extern __IO uint8_t flag__dma1_channel3_done;
extern uint8_t ffrank_buffer[];

int main(void) {

	// System clock = PCLK1 = PCLK2 = 64MHz
	System_ClockInit();

	TIM_Delay_Init();

	// Baud rate: 57600
	UART_Init();

	stm32_printf("ST7735 - Debug monitor\r\n");

	ST7735_Init();

	// Print ST7735 ID1 (Manufacturer ID), ID2 (driver version ID), ID3 (driver ID)
	uint8_t id_buffer[3] = {0};

	ST7735_ReadID(id_buffer, ALL_IDs);

	// Manufacturer ID should be 0x7C or 124 by default
	stm32_printf("[INFO] Manufacturer ID : %d\r\n", id_buffer[0]);
	stm32_printf("[INFO] Driver version ID : %d\r\n", id_buffer[1]);
	stm32_printf("[INFO] Driver ID : %d\r\n", id_buffer[2]);

	ST7735_SetBacklight(BL_ON);

	// Enable Interrupts
	ST7735_NVIC_Init();

	// Fill the LCD RAM with data from st7735_frame.c
	ST7735_MemoryWriteDMA();

	// Draw some rectangles
	// Note that last row / columns index is included
	while(flag__dma1_channel3_done == 0);
	ST7735_DrawRectangle(10, 10, 19, 19, RED_666);
	ST7735_DrawRectangle(20, 20, 29, 29, GREEN_666);
	ST7735_DrawRectangle(30, 30, 39, 39, BLUE_666);

	// Write 40x40 pixel image at position (50,50)
	ST7735_MemoryWrite(ffrank_buffer, 40, 40, 50, 50);

	// Mirror in X, not in Y
	ST7735_SetMirror(1, 0);

	// Write same 40x40 pixel image at position (50,100), should be flipped
	// Note that x' <= 128 - x - frame_x_size
	ST7735_MemoryWrite(ffrank_buffer, 40, 40, 128-50-40, 100);

	while(1) {

	}
	return 0;
}

static void System_ClockInit(void) {
	// TARGET : 64MHz Core clock frequency
	//
	// HSI is 16MHz and F(SYSCLK) = F(HSI) * PLLN / (PLLR * PLLM)
	// For example :
	//    - PLLN = 8
	//    - PLLR = 2 (actual value is 0)
	//    - PLLM = 1 (actual value is 0)
	//
	// AHB, APB1 and APB2 pre-scalers are /1 (default)

	uint32_t timeout = 0;

	// Turn on HSI
	RCC->CR |= RCC_CR_HSION;

	// Wait for HSE to settle
	timeout = 100000;
	while((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY && --timeout > 0);

	// Disable PLL so that it can be modified
	RCC->CR &= ~RCC_CR_PLLON_Msk;
	timeout = 100000;
	while((RCC->CR & RCC_CR_PLLRDY) == RCC_CR_PLLRDY && --timeout > 0);

	// Select HSI as PLL input source => PLL clock input is 16MHz
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC_Msk;
	RCC->PLLCFGR |= (0x02 << RCC_PLLCFGR_PLLSRC_Pos);

	// Set PLL parameters for 64MHz output
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN_Msk | RCC_PLLCFGR_PLLM_Msk | RCC_PLLCFGR_PLLR_Msk);
	RCC->PLLCFGR |= (0x08 << RCC_PLLCFGR_PLLN_Pos);

	// Turn PLL back on
	RCC->CR |= RCC_CR_PLLON;
	timeout = 100000;
	while((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY && --timeout > 0);

	// Enable PLL main output (/R output)
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;

	// For the FLASH, enable PREFETCH (instruction PREFETCH ?) and set correct LATENCY (3 wait states)
	// Enable FLASH clock
	RCC->AHB1ENR |= RCC_AHB1ENR_FLASHEN;
	FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
	FLASH->ACR |= (0x03 << FLASH_ACR_LATENCY_Pos) | FLASH_ACR_PRFTEN;

	// Switch to PLL for SYSCLK
	RCC->CFGR &= ~RCC_CFGR_SW_Msk;
	RCC->CFGR |= (0x03 << RCC_CFGR_SW_Pos);
	timeout = 100000;
	while((RCC->CFGR & (0x03 << RCC_CFGR_SWS_Pos)) != RCC_CFGR_SWS_PLL && --timeout > 0);

	// Update global variable
	SystemCoreClockUpdate();
}
