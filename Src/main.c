#include "stm32l476xx.h"
#include "SERVO.h"
#include "LED.h"
#include "USART1.h"
#include "I2C.h"

uint16_t raw = 0;
float lux = 0.0;

void FPU_Enable(void)
{
    SCB->CPACR |= ((3UL << 20U) | (3UL << 22U));
    __DSB();
    __ISB();
}

int main(void)
{
	// 0. Enable floating point operations
	FPU_Enable();

	// 1. Invoke configure_PA1() to initialize PA1 as TIM5_CH2 output signal, interfacing with the servo motor.
	configure_PA1();

	// 2. Invoke TIM5_CH2_Init() to configure and initialize TIM5 Channel 2 to function in Output PWM Mode 1
	TIM5_CH2_Init();

	// 3.  Center the servo
	Servo_Move_Center_All();

	// 4. Invoke configure_LED_pin() to initialize PA5 as an output pin, interfacing with the LD2 LED.
	configure_LED_pin();

	// 5. Initialize USART1 for communication with the blue-tooth module
	// Configuration details:
	// - Data format: 8 data bits, no parity, 1 start bit, and 1 stop bit
	// - Baud rate: 9600
	USART1_Init();

	// 6. 0xF0420F13U
	I2C_GPIO_init();
	I2C_Initialization(I2C1, 0x10100909U);

	VEML7700_WriteReg16(VEML7700_ALS_CONF_0, 0x0000);
	// Configure VEML7700
	// 0x0000 means:
	// Gain = 1x
	// Integration time = 100 ms
	// Interrupt disabled
	// Shutdown = 0, sensor enabled


	while(1)
	{
		raw = VEML7700_ReadReg16(VEML7700_ALS_DATA);
		// Only calculate lux if read did not fail
		if (raw != 0xFFFF)
		{
			// For VEML7700:
			// Gain = 1x
			// Integration time = 100 ms
			// Resolution = 0.0576 lux/count
			lux = raw * 0.0576f;
		}


	}
}

