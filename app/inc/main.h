/*
 * main.h
 *
 *  Created on: 7 févr. 2024
 *      Author: anton
 */

#ifndef APP_INC_MAIN_H_
#define APP_INC_MAIN_H_

#include "stm32l4xx.h"
#include "uart.h"
#include "delay.h"
#include "st7735.h"

// Functions from smallprintf.c
extern int stm32_printf(const char *format, ...);
extern int stm32_sprintf(char *out, const char *format, ...);

#endif /* APP_INC_MAIN_H_ */
