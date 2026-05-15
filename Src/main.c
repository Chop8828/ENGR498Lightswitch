#include "stm32l476xx.h"
#include "SERVO.h"
#include "LED.h"
#include "USART1.h"
#include "I2C.h"
#include "SysTick.h"
#include <string.h>
#include <stdlib.h>
#include <string.h>

#define LIGHT_OFF_VERIFY_DELAY_MS   300U
#define LIGHT_OFF_MAX_ATTEMPTS      5U

#define LIGHT_SAMPLE_DELAY_MS       100U

// raw lux values
volatile uint16_t raw = 0;
volatile float lux = 0.0f;

// calculated ambient values
volatile uint16_t ambient_baseline_raw = 0;
volatile uint16_t light_on_threshold_raw = 0;
volatile uint16_t light_off_threshold_raw = 0;
volatile uint16_t light_off_drop_raw = 0;

volatile uint8_t light_state = 0; // global: 0 = off, 1 = on

volatile uint8_t keep_on_active = 0;
volatile uint32_t keep_on_end_ms = 0;

void FPU_Enable(void) {
	SCB->CPACR |= ((3UL << 20U) | (3UL << 22U));
	__DSB();
	__ISB();
}

void delay_ms(uint32_t ms) //4mhz sysclk
{
	for (uint32_t i = 0; i < ms; i++) {
		for (volatile uint32_t j = 0; j < 4000; j++) {
			__NOP();
		}
	}
}

float calculate_lux(uint16_t raw_value) {
	/*
	 * ALS_CONF_0 = 0x0000
	 * Gain = 1x
	 * Integration time = 100 ms
	 * Resolution ~= 0.0576 lux/count
	 */
	return raw_value * 0.0576f;
}

// set lux thresholds to match room
void Ambient_CalibrateStartup(void) {
	uint32_t sum = 0;
	uint16_t sample = 0;
	uint32_t valid_samples = 0;

	const uint32_t samples = 10;

	delay_ms(200);

	for (uint32_t i = 0; i < samples; i++) {
		sample = VEML7700_ReadReg16(VEML7700_ALS_DATA);

		if (sample != 0xFFFF) {
			sum += sample;          // raw counts, not lux
			valid_samples++;
		}

		delay_ms(200);
	}

	if (valid_samples == 0) {
		ambient_baseline_raw = 0;
		light_on_threshold_raw = 100;
		light_off_threshold_raw = 200;
		light_off_drop_raw = 50;
		return;
	}

	ambient_baseline_raw = sum / valid_samples;

	light_on_threshold_raw = ambient_baseline_raw / 2U;
	light_off_threshold_raw = (ambient_baseline_raw * 8U) / 10U;

	if (light_on_threshold_raw < 50U) {
		light_on_threshold_raw = 50U;
	}

	if (light_off_threshold_raw <= light_on_threshold_raw) {
		light_off_threshold_raw = light_on_threshold_raw + 100U;
	}

	light_off_drop_raw = ambient_baseline_raw / 4U;

	if (light_off_drop_raw < 50U) {
		light_off_drop_raw = 50U;
	}
}

uint8_t Light_Is_Actually_Off(uint16_t starting_raw, uint16_t current_raw) {
	if (current_raw == 0xFFFF) {
		return 0;
	}

	if (current_raw < light_on_threshold_raw) {
		return 1;
	}

	if ((starting_raw > current_raw)
			&& ((starting_raw - current_raw) >= light_off_drop_raw)) {
		return 1;
	}

	return 0;
}

uint8_t Light_Reached_Target(uint16_t starting_raw, uint16_t current_raw,
		char value[]) {
	if (current_raw == 0xFFFF) {
		return 0;
	}

	if (strcmp(value, "off") == 0) {
		if (current_raw < light_on_threshold_raw) {
			return 1;
		}

		if ((starting_raw > current_raw)
				&& ((starting_raw - current_raw) >= light_off_drop_raw)) {
			return 1;
		}
	} else if (strcmp(value, "on") == 0) {
		if (current_raw > light_off_threshold_raw) {
			return 1;
		}

		if ((current_raw > starting_raw)
				&& ((current_raw - starting_raw) >= light_off_drop_raw)) {
			return 1;
		}
	}

	return 0;
}

uint8_t Try_Turn_Light_Until_Target(char value[]) {
	uint16_t starting_raw = 0;
	uint16_t current_raw = 0;
	uint32_t attempts = 0;

	// Store the lux/raw value from when the light is currently on
	starting_raw = VEML7700_ReadReg16(VEML7700_ALS_DATA);

	if (starting_raw == 0xFFFF) {
		return 0; // sensor read failed
	}

	while (attempts < LIGHT_OFF_MAX_ATTEMPTS) {
		if (strcmp(value, "on") == 0) {
			Servo_Move_On_FlickSwitch();
			turn_on_LED();
		} else if (strcmp(value, "off") == 0) {
			Servo_Move_Off_FlickSwitch();
			turn_off_LED();
		} else {
			return 0;
		}

		delay_ms(LIGHT_OFF_VERIFY_DELAY_MS);

		current_raw = VEML7700_ReadReg16(VEML7700_ALS_DATA);

		if (Light_Reached_Target(starting_raw, current_raw, value)) {
			raw = current_raw;
			lux = calculate_lux(current_raw);

			if (strcmp(value, "on") == 0) {
				light_state = 1;
			} else if (strcmp(value, "off") == 0) {
				light_state = 0;
			}

			return 1;
		}

		attempts++;
	}

	return 0; // tried several times, but did not observe light drop
}

void keep_on_x_time(char value[]) {
	uint32_t minutes = 0;
	uint32_t duration_ms = 0;

	minutes = (uint32_t) atoi(value);

	if (minutes == 0) {
		keep_on_active = 0;
		return;
	}

	duration_ms = minutes * 60U * 1000U;

	Try_Turn_Light_Until_Target("on");

	keep_on_active = 1;
	keep_on_end_ms = millis() + duration_ms;
}

uint8_t keep_on_time_expired(void) {
	if (keep_on_active == 0) {
		return 0;
	}

	if ((int32_t) (millis() - keep_on_end_ms) >= 0) {
		keep_on_active = 0;
		return 1;
	}

	return 0;
}

int main(void) {
	FPU_Enable();
	SysTick_Init();

	configure_PA1();
	TIM5_CH2_Init();

	Servo_Move_Center_All();

	configure_LED_pin();

	USART1_Init();

	I2C_GPIO_init();
	I2C_Initialization(I2C1, 0x10100909U);

	/*
	 * VEML7700 config:
	 * 0x0000 =
	 * Gain = 1x
	 * Integration time = 100 ms
	 * Interrupt disabled
	 * Sensor enabled
	 */
	VEML7700_WriteReg16(VEML7700_ALS_CONF_0, 0x0000);

	delay_ms(200);

	//NOTE: ROOM NEEDS TO HAVE LIGHT OFF SO AMBIENT IS ONLY CALCULATED BASED OFF ACTUAL AMBIENT LIGHT
	Ambient_CalibrateStartup();

	while (1) {
		raw = VEML7700_ReadReg16(VEML7700_ALS_DATA);

		if (bluetooth_button == '1') {
			keep_on_active = 0;

			Try_Turn_Light_Until_Target("on");

			bluetooth_button = '\0';
			delay_ms(LIGHT_SAMPLE_DELAY_MS);
			continue;
			// dont want ambient light logic messing with manual flicking
		} else if (bluetooth_button == '0') {
			keep_on_active = 0;

			Try_Turn_Light_Until_Target("off");

			bluetooth_button = '\0';
			delay_ms(LIGHT_SAMPLE_DELAY_MS);
			continue;
		} else if (keep_on_active) {
			if (keep_on_time_expired()) {
				delay_ms(LIGHT_SAMPLE_DELAY_MS);
				continue;
			}

			if (light_state == 0) {
				Try_Turn_Light_Until_Target("on");
			}

			delay_ms(LIGHT_SAMPLE_DELAY_MS);
			continue;
		} else if ((raw < light_on_threshold_raw) && (light_state == 0)) {
			turn_off_LED();
			Try_Turn_Light_Until_Target("on");
		} else if ((raw > light_off_threshold_raw) && (light_state == 1)) {
			Try_Turn_Light_Until_Target("off");
		}
	}
}
