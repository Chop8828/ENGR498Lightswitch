/*
 * Servo.h
 *
 *  Created on: Apr 18, 2026
 *      Author: Dylan Thai
 */

#ifndef __STM32L476G_SERVO_H
#define __STM32L476G_SERVO_H

#include "stm32l476xx.h"

// Configure PA1 as TIM5_CH2 (Alternate Function) for PWM output
void configure_PA1();

// Initialize TIM5 Channel 2 for PWM generation (used for servo control)
void TIM5_CH2_Init();

// Move servo to center position
void Servo_Move_Center_All();

///////////////////////////////////////////////////////////////////////
// FlickSwitch
///////////////////////////////////////////////////////////////////////

// Move servo to approximately -90 degrees
void Servo_Move_Off_FlickSwitch();

// Move servo to approximately +90 degrees
void Servo_Move_On_FlickSwitch();

// Test the movement of servo for a flick-switch
void test_Servo_FlickSwitch();

///////////////////////////////////////////////////////////////////////
// Button
///////////////////////////////////////////////////////////////////////

void Servo_Move_Press_Button();

void test_Servo_Button();


#endif /* SERVO_H_ */
