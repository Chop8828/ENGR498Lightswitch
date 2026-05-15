/*
 * Uart1.c
 *
 *  Created on: May 1, 2026
 *      Author: Dylan
 */

#include "stm32l476xx.h"
#include "USART1.h"
#include "LED.h"
#include "SERVO.h"

#include <stdlib.h>
#include <string.h>


// UART Ports:
// ===================================================
// PA9 = USART1_TX (AF7)
// PA10= USART1_RX (AF7)

#define TX_PIN 9
#define RX_PIN 10

volatile uint8_t bluetooth_button;
volatile uint8_t bluetooth_data_ready;

static volatile char bluetooth_rx_buffer[BT_CMD_MAX_LEN];
static volatile uint32_t bluetooth_rx_index = 0;
static volatile uint8_t bluetooth_command_ready = 0;

static char bluetooth_command_buffer[BT_CMD_MAX_LEN];

static void USART1_WriteChar(char c)
{
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = c;
}

void USART1_WriteString(const char *s)
{
    while (*s)
    {
        USART1_WriteChar(*s++);
    }
}

// This function initializes the GPIO pins used for USART1 communication.
void USART1_Pin_Init(void)
{
	// Enable the clock for GPIO Port A to allow pin configuration.
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// Clear the mode bits for PA9 and PA10.
    GPIOA->MODER &= ~((3U << (2 * TX_PIN)) | (3U << (2 * RX_PIN)));

    // Set PA9 and PA10 to Alternate Function mode ('10').
    GPIOA->MODER |=  ((2U << (2 * TX_PIN)) | (2U << (2 * RX_PIN)));

    // Clear alternate function selection bits for PA9 and PA10
    GPIOA->AFR[1] &= ~((0xFU << (4 * (TX_PIN - 8))) | (0xFU << (4 * (RX_PIN - 8))));

    // Set alternate function 7 (USART1) for PA9 and PA10
    GPIOA->AFR[1] |=  ((7U   << (4 * (TX_PIN - 8))) | (7U   << (4 * (RX_PIN - 8))));
}

void USART1_Init(void)
{
	// data format to be set: 8 data bits, no parity, 1 start bit, and 1 stop bit
	// baud rate to be set: 9600

	// Enable the USART1 clock in the APB2 peripheral clock enable register
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	// Initialize the TX and RX pins for USART1 communication
	USART1_Pin_Init();

	// Disabling USART to allow configuration
    USART1->CR1 &= ~USART_CR1_UE;

    // Configuring word length: 8 data bits
    // M bit settings: 00 = 8 data bits, 01 = 9 data bits, 10 = 7 data bits
    USART1->CR1 &= ~USART_CR1_M0;

    // Configuring the number of stop bits: 1 stop bit
    // STOP bits settings: 00 = 1 Stop bit, 01 = 0.5 Stop bit, 10 = 2 Stop bits, 11 = 1.5 Stop bits
    USART1->CR2 &= ~USART_CR2_STOP;

    // Setting the oversampling mode to 16
    // OVER8 = 0 for oversampling by 16, 1 for oversampling by 8
    USART1->CR1 &= ~USART_CR1_OVER8;

    // Setting the baud rate to 9600 using the default APB frequency of 4,000,000 Hz
    // Calculation depends on the oversampling mode: if OVER8 = 0 (16x), BRR = f_CK / USARTDIV
    // For 9600 baud with f_CK = 4,000,000 Hz and 16x oversampling: USARTDIV = 4,000,000 / 9,600 = 417
    USART1->BRR = 417;

    // Enabling the transmitter and receiver
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;

    // Enabling the RXNE (Receiver Data Register (RDR) Not Empty) interrupt
    USART1->CR1 |= USART_CR1_RXNEIE;

    // Enabling the USART1 interrupt in the NVIC
    NVIC_EnableIRQ(USART1_IRQn);

    // Enabling USART after configuration is completed
    USART1->CR1 |= USART_CR1_UE;

	// Ensuring the USART is ready for transmission and reception
	// Wait for TEACK: Transmitter Enable Acknowledge Flag
    while ((USART1->ISR & USART_ISR_TEACK) == 0);

	// Wait for REACK: Receiver Enable Acknowledge Flag
    while ((USART1->ISR & USART_ISR_REACK) == 0);
}

void USART1_BluetoothParser_Init(void) {
    bluetooth_button = 0;
    bluetooth_data_ready = 0;

    bluetooth_rx_index = 0;
    bluetooth_command_ready = 0;

    for (uint32_t i = 0; i < BT_CMD_MAX_LEN; i++) {
        bluetooth_rx_buffer[i] = '\0';
        bluetooth_command_buffer[i] = '\0';
    }
}

uint8_t USART1_GetBluetoothCommand(BluetoothCommand *cmd) {
    if (bluetooth_command_ready == 0) {
        return 0;
    }

    __disable_irq();

    for (uint32_t i = 0; i < BT_CMD_MAX_LEN; i++) {
        bluetooth_command_buffer[i] = bluetooth_rx_buffer[i];
    }

    bluetooth_command_ready = 0;

    __enable_irq();

    cmd->type = BT_CMD_INVALID;
    cmd->minutes = 0;

    if (strcmp(bluetooth_command_buffer, "1") == 0) {
        cmd->type = BT_CMD_ON;
    }
    else if (strcmp(bluetooth_command_buffer, "0") == 0) {
        cmd->type = BT_CMD_OFF;
    }
    else if (strncmp(bluetooth_command_buffer, "KEEP ", 5) == 0) {
        cmd->type = BT_CMD_KEEP_ON;
        cmd->minutes = (uint32_t)atoi(&bluetooth_command_buffer[5]);

        if (cmd->minutes == 0) {
            cmd->type = BT_CMD_INVALID;
        }
    }
    else {
        cmd->type = BT_CMD_INVALID;
    }

    return 1;
}

// This function serves as the interrupt handler for USART1.
void USART1_IRQHandler(void)
{
    // Check if the RXNE (Receive Not Empty) interrupt is triggered, indicating new data is available in the USART_RDR register.
    if (USART1->ISR & USART_ISR_RXNE)
    {
    	// Read data from the receiver data register (RDR), which also clears the RXNE flag.
    	USART1->ICR |= USART_ICR_ORECF;

    	// Read data from the receiver data register (RDR), which also clears the RXNE flag.
    	bluetooth_button = (uint8_t)USART1->RDR;
    	bluetooth_data_ready = 1;

        if (bluetooth_button == '1' || bluetooth_button == '0') {
            bluetooth_rx_buffer[0] = (char)bluetooth_button;
            bluetooth_rx_buffer[1] = '\0';
            bluetooth_rx_index = 0;
            bluetooth_command_ready = 1;
            return;
        }

        if (bluetooth_button == '\r' || bluetooth_button == '\n')
        {
            if (bluetooth_rx_index > 0) {
                bluetooth_rx_buffer[bluetooth_rx_index] = '\0';
                bluetooth_command_ready = 1;
            }

            bluetooth_rx_index = 0;
            return;
        }

        if (bluetooth_rx_index < (BT_CMD_MAX_LEN - 1U)) {
            bluetooth_rx_buffer[bluetooth_rx_index] = (char)bluetooth_button;
            bluetooth_rx_index++;
        }
        else {
            bluetooth_rx_index = 0;
            bluetooth_command_ready = 0;
        }
    }
}
