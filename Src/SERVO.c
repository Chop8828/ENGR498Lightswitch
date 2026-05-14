/*
 * SERVO.c
 *
 *  Created on: Apr 18, 2026
 *      Author: Chop
 */

#include "SERVO.h"

#define PA1  1

// PA1  <--> SERVO

void configure_PA1()
{
    // 1. Enable the clock to GPIO Port A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// 2. Configure GPIO Mode to 'Alternative Function' mode: Input(00), Output(01), Alternative Function(10), Analog(11)
	GPIOA->MODER &= ~(0b11<<(2*PA1));
	GPIOA->MODER |=   0b10<<(2*PA1);      // Alternative Function(10)

	// 3. Select PA1's alternative function as TIM5_CH2, which is 'AF2', by configuring the GPIOA_AFRL register
	GPIOA->AFR[0] &= ~(0b1111 << (4*PA1));
	GPIOA->AFR[0] |= 0b0010 << (4*PA1); // AF2

	// 4. Configure GPIO Output Type to 'Push-Pull': Output push-pull (0), Output open drain (1)
	GPIOA->OTYPER &= ~(1<<PA1);      // Push-pull

	// 5. Configure GPIO Push-Pull to 'No Pull-up or Pull-down': No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOA->PUPDR  &= ~(0b11<<(2*PA1));  // No pull-up, no pull-down
}

void TIM5_CH2_Init()
{
    //1. Enable the clock to TIM5 by configuring RCC_APB1ENR1 register
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;

    //2. Configure TIM5 counting mode to upcounting (TIMx_CR1)
	TIM5->CR1 &= ~(0b1 << 4);

	//3. Configure TIM5 Prescalar (TIMx_PSC) and ARR (TIMx_ARR) to generate a counter period of 20 ms
	TIM5->PSC = 1;
	TIM5->ARR = 39999;

	//4. Configure TIMx_CCMR1 CC2S[1:0] bits to output mode for TIM5 Channel 2
	TIM5->CCMR1 &= ~(0b11 << 8);

	//5. Configure TIMx_CCMR1 OC2M[3:0] bits to PWM mode 1 ('0110') for TIM5 Channel 2
	TIM5->CCMR1 &= ~(0b111U << 12); // clear bits 14:12
	TIM5->CCMR1 |=  (0b110U << 12); // set bits 14:12 to 110 (PWM mode 1)

    //6. Set TIMx_CCER CC2E bit to enable output signal on Channel 2
	TIM5->CCER &= ~(0b1 << 4);
	TIM5->CCER |=   0b1 << 4;

	//7. Set TIMx_CR1 CEN bit to enable the TIM2 counter
	TIM5->CR1 &= ~(0b1);
	TIM5->CR1 |=   0b1;
}

void Servo_Move_Center_All()
{
	TIM5->CCR2 = 3000;
}

///////////////////////////////////////////////////////////////////////
// FlickSwitch
///////////////////////////////////////////////////////////////////////

void Servo_Move_Off_FlickSwitch()
{
	TIM5->CCR2 = 2200;
}

void Servo_Move_On_FlickSwitch()
{
	TIM5->CCR2 = 3800;
}

void test_Servo_FlickSwitch()
{
	Servo_Move_Center_All();
	for(volatile int i = 0; i < 2000000; i++);  // delay

	Servo_Move_Off_FlickSwitch();
	for(volatile int i = 0; i < 2000000; i++);  // delay

	Servo_Move_On_FlickSwitch();
	for(volatile int i = 0; i < 2000000; i++);  // delay
}

///////////////////////////////////////////////////////////////////////
// Button
///////////////////////////////////////////////////////////////////////

void Servo_Move_Press_Button()
{
	TIM5->CCR2 = 4000;
}

void test_Servo_Button()
{
	Servo_Move_Center_All();
	for(volatile int i = 0; i < 2000000; i++);  // delay
	Servo_Move_Press_Button();
	for(volatile int i = 0; i < 50000; i++);  // delay
}

///////////////////////////////////////////////////////////////////////
// Switch
///////////////////////////////////////////////////////////////////////



