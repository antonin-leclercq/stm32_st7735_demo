/*
 * uart.c
 *
 *  Created on: Feb 9, 2024
 *      Author: anton
 */

#include "uart.h"

void UART_Init(void) {
	// Using USART2 peripheral (APB1)
	// Peripheral input clock is 64MHz
	//
	// Pin description:
	//    - PA2 : USART2_TX (AF7)
	//    - PA3 : USART2_RX (AF7)
	//
	// USART2 configuration : 8n1
	// Baud rate : 57600 => USARTDIV = 1111 (base 10)

	// Configure GPIOs

	// Enable GPIOA clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// Set pins PA2 and PA3 to AF7
	GPIOA->MODER &= ~(GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
	GPIOA->MODER |= (0x02 << GPIO_MODER_MODE2_Pos) | (0x02 << GPIO_MODER_MODE3_Pos);
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk);
	GPIOA->AFR[0] |= (0x07 << GPIO_AFRL_AFSEL2_Pos) | (0x07 << GPIO_AFRL_AFSEL3_Pos);

	// Enable USART2 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

	// Reset USART2 configuration
	USART2->CR1 = 0x00000000;
	USART2->CR2 = 0x00000000;
	USART3->CR3 = 0x00000000;

	// Set baud rate
	USART2->BRR = (uint32_t)1111;

	// Enable transmitter and receiver
	USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;

	// Enable USART2
	USART2->CR1 |= USART_CR1_UE;

}
