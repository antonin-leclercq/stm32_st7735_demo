/*
 * main.c
 *
 *  Created on: 6 févr. 2024
 *      Author: anton
 */


#include "main.h"

static void System_ClockInit(void);

int main(void) {

	// System clock = PCLK1 = PCLK2 = 64MHz
	System_ClockInit();

	TIM_Delay_Init();

	UART_Init();

	stm32_printf("ST7735 - Debug monitor\r\n");

	ST7735_Init();

	// Print ST7735 ID1 (Manufacturer ID)
	uint8_t id1_buffer = 0;

	ST7735_ReadID(&id1_buffer, ID1);

	// Manufacturer ID should 0x7C or 124 by default
	stm32_printf("[INFO] Manufacturer ID : %d\r\n", id1_buffer);

	ST7735_SetBacklight(ON);

	// Enable Interrupts
	ST7735_NVIC_Init();

	// Fill the LCD RAM with data from st7735_frame.c
	ST7735_MemoryWriteDMA();

	// Or do it without the DMA
	// ST7735_MemoryWrite();

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
