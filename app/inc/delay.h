/*
 * delay.h
 *
 *  Created on: Feb 11, 2024
 *      Author: anton
 */

#ifndef APP_INC_DELAY_H_
#define APP_INC_DELAY_H_

#include "stm32l4xx.h"

void TIM_Delay_Init(void);

void TIM_Delay_Milli(const uint32_t t);
void TIM_Delay_Micro(const uint32_t t);

#endif /* APP_INC_DELAY_H_ */
