/*
 * UART1.h
 *
 *  Created on: May 1, 2026
 *      Author: Dylan
 */

#ifndef __STM32L476G_USART1_H
#define __STM32L476G_USART1_H

#include "stm32l476xx.h"

#define BT_CMD_MAX_LEN  20U

// enum for command states
typedef enum {
    BT_CMD_NONE = 0,
    BT_CMD_ON,
    BT_CMD_OFF,
    BT_CMD_KEEP_ON,
    BT_CMD_INVALID
} BluetoothCommandType;

// struct for command values
typedef struct {
    BluetoothCommandType type;
    uint32_t minutes;
} BluetoothCommand;

extern volatile uint8_t bluetooth_button;
extern volatile uint8_t bluetooth_data_ready;

void USART1_Init(void);
void USART1_Pin_Init(void);
void USART1_WriteString(const char *s);

void USART1_BluetoothParser_Init(void);
uint8_t USART1_GetBluetoothCommand(BluetoothCommand *cmd);

#endif /* __STM32L476G_USART1_H */
