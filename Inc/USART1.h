/*
 * UART1.h
 *
 *  Created on: May 1, 2026
 *      Author: Dylan
 */

#ifndef __STM32L476G_USART1_H
#define __STM32L476G_USART1_H

#include "stm32l476xx.h"

extern volatile uint8_t bluetooth_button;
extern volatile uint8_t bluetooth_data_ready;

void USART1_WriteString(const char *s);

// This function initializes the USART1 module
void USART1_Init(void);

// This function initializes the GPIO pins used for USART1 communication.
void USART1_Pin_Init(void);

// This function initializes USART module with specified settings for communication.
// This function is modular and can be utilized with any USART module passed as an argument.
void USART_Init(USART_TypeDef * USARTx);

#endif /* __STM32L476G_USART1_H */
