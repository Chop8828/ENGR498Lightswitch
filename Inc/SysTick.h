/*
 * SysTick.h
 *
 *  Created on: May 14, 2026
 *      Author: trenton
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "stm32l476xx.h"

void SysTick_Init(void);
uint32_t millis(void);

#endif /* SYSTICK_H_ */
