/*
 * delay.c
 *
 *  Created on: Feb 11, 2024
 *      Author: anton
 */

#include "delay.h"

void TIM_Delay_Init(void) {
	// Start TIM6 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;

	// Reset Timer 6 configuration
	TIM6->CR1 |= 0x0000;

	// Auto-reload pre-load enable (timer is buffered)
	TIM6->CR1 |= TIM_CR1_ARPE;

	// Set the timer auto-reload to max (default)
	TIM6->ARR = 0xFFFF;

	// Disable Timer 6 for the moment
	TIM6->CR1 &= ~TIM_CR1_CEN;
}

void TIM_Delay_Milli(const uint32_t t) {
	// Set the timer pre-scaler for 1kHz counting frequency
	TIM6->PSC = (uint16_t)64000 -1;

	// Reset Timer 6
	TIM6->EGR |= TIM_EGR_UG;

	// Enable Timer 6
	TIM6->CR1 |= TIM_CR1_CEN;

	// Wait for timer
	while(TIM6->CNT < t);

	// Disable Timer 6
	TIM6->CR1 &= ~TIM_CR1_CEN;
}

void TIM_Delay_Micro(const uint32_t t) {
	// Set the timer pre-scaler for 1MHz counting frequency
	TIM6->PSC = (uint16_t)64 -1;

	// Reset Timer 6
	TIM6->EGR |= TIM_EGR_UG;

	// Enable Timer 6
	TIM6->CR1 |= TIM_CR1_CEN;

	// Wait for timer
	while(TIM6->CNT < t);

	// Disable Timer 6
	TIM6->CR1 &= ~TIM_CR1_CEN;
}
