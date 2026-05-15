/*
 * SysTick.c
 *
 *  Created on: May 14, 2026
 *      Author: trenton
 */


#include "SysTick.h"

volatile uint32_t systick_ms = 0;

void SysTick_Init(void) {
    SysTick->LOAD = 4000 - 1;    // 1 ms tick with 4 MHz sysclk
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void) {
    systick_ms++;
}

uint32_t millis(void) {
    return systick_ms;
}
