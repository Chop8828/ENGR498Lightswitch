/*
 * LED.c
 *
 *  Created on: Apr 18, 2026
 *      Author: Dylan Thai
 */

#include "LED.h"

#define LED_PIN  5

// PA2  <--> LED

void configure_LED_pin()
{
	// 1. Enable the clock to GPIO Port A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// 2. Configure GPIO Mode to 'Output': 01
	GPIOA->MODER &= ~(3UL<<(2*LED_PIN));
	GPIOA->MODER |=   1UL<<(2*LED_PIN);		// Output(01)

	// 3. Configure GPIO Output Type to 'Push-Pull': 0
	GPIOA->OTYPER &= ~(1<<LED_PIN);       // Push-pull

	// 4. Configure GPIO Push-Pull to 'No Pull-up or Pull-down': 00
	GPIOA->PUPDR  &= ~(3<<(2*LED_PIN));   // No pull-up, no pull-down
}

//Turn on the PA2 LED
void turn_on_LED()
{
	GPIOA->ODR |= 1 << LED_PIN;
}

//Turn of the PA2 LED
void turn_off_LED()
{
	GPIOA->ODR &= ~(1 << LED_PIN);
}

//Toggle the PA2 LED
void toggle_LED()
{
	GPIOA->ODR ^= (1 << LED_PIN);
}

//Blink the PA2 LED
void blink()
{
	turn_on_LED();
	turn_off_LED();
	turn_on_LED();
	turn_off_LED();
}
