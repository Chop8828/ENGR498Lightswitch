/*
 * LED.h
 *
 *  Created on: Apr 18, 2026
 *      Author: Dylan Thai
 */

#ifndef __STM32L476G_LED_H
#define __STM32L476G_LED_H

#include "stm32l476xx.h"

//Configure the PA1 as an output pin for a LED
void configure_LED_pin();

//Turn on the PA1 LED
void turn_on_LED();

//Turn of the PA1 LED
void turn_off_LED();

//Toggle the PA1 LED
void toggle_LED();

#endif /* LED_H_ */
