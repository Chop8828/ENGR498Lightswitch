#ifndef __STM32L476G_DISCOVERY_I2C_H
#define __STM32L476G_DISCOVERY_I2C_H

#include <stdint.h>
#include <stddef.h>
#include "stm32l476xx.h"

#define READ_FROM_PERIPHERAL 1
#define WRITE_TO_PERIPHERAL  0

#define PB8	8
#define PB9	9
#define I2C1_SCL	PB8 // PB8 = I2C1_SCL (AF4)
#define I2C1_SDA	PB9	// PB9 = I2C1_SDA (AF4)

#define VEML7700_ADDR_7BIT      0x10

#define VEML7700_ALS_CONF_0     0x00
#define VEML7700_ALS_WH         0x01
#define VEML7700_ALS_WL         0x02
#define VEML7700_POWER_SAVE     0x03
#define VEML7700_ALS_DATA       0x04
#define VEML7700_WHITE_DATA     0x05
#define VEML7700_INTERRUPT      0x06

void I2C_GPIO_init(void);

int8_t I2C_Start(I2C_TypeDef * I2Cx, uint32_t DevAddress, uint8_t Size, uint8_t Direction);

//void I2C_Stop(I2C_TypeDef * I2Cx);

void I2C_WaitLineIdle(I2C_TypeDef * I2Cx);

int8_t I2C_SendData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size);

int8_t I2C_ReceiveData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size);

void I2C_Initialization(I2C_TypeDef * I2Cx, uint32_t TimingConfig);

int8_t VEML7700_WriteReg16(uint8_t reg, uint16_t value);

uint16_t VEML7700_ReadReg16(uint8_t reg);

#endif /* __STM32L476G_DISCOVERY_I2C_H */
