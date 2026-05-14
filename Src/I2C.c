#include "I2C.h"

extern void Error_Handler(void);

// Inter-integrated Circuit Interface (I2C)
// up to 100 Kbit/s in the standard mode,
// up to 400 Kbit/s in the fast mode, and
// up to 3.4 Mbit/s in the high-speed mode.

// Recommended external pull-up resistance is
// 4.7 kOmh for low speed,
// 3.0 kOmh for the standard mode, and
// 1.0 kOmh for the fast mode

//=============================================================================================================
//                          I2C Pin Initialization
// Configure I2C1_SCL (PB8) Pin as : Alternate function (AF4), High Speed, Open drain, No Pull up or Pull down
// Configure I2C1_SDA (PB9) Pin as : Alternate function (AF4), High Speed, Open drain, No Pull up or Pull down
//=============================================================================================================
void I2C_GPIO_init(void)
{

	RCC->AHB2ENR  |=  RCC_AHB2ENR_GPIOBEN;

	// GPIO Mode: Input(00, reset), Output(01), AlterFunc(10), Analog(11, reset)
	GPIOB->MODER   &= ~( 3U<<(2*I2C1_SCL) | 3U<<(2*I2C1_SDA) );  // Clear Mode
	GPIOB->MODER   |=    2U<<(2*I2C1_SCL) | 2U<<(2*I2C1_SDA);    // Alternative Function


	//GPIOB->AFR[0]  |= 0x44000000;   // Alternative Function 4 = I2C1
	GPIOB->AFR[1]  |= 4U<<(4*(I2C1_SCL-8));   // Alternative Function 4 = I2C1
	GPIOB->AFR[1]  |= 4U<<(4*(I2C1_SDA-8));   // Alternative Function 4 = I2C1

	// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)
	GPIOB->OSPEEDR |=    3U<<(2*I2C1_SCL) | 3U<<(2*I2C1_SDA);    // High Speed

	// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOB->PUPDR   &= ~( 3U<<(2*I2C1_SCL) | 3U<<(2*I2C1_SDA) );
	//GPIOB->PUPDR   |=    1U<<(2*I2C1_SCL) | 1U<<(2*I2C1_SDA) ;   // Pull-up

	// GPIO Output Type: Output push-pull (0, reset), Output open drain (1)
	GPIOB->OTYPER  |= 1U<<I2C1_SCL | 1U<<I2C1_SDA;  // Open Drain
}

//===============================================================================
//                          I2C Initialization
//===============================================================================
void I2C_Initialization(I2C_TypeDef * I2Cx, uint32_t TimingConfig)
{

	// Enable the clock of I2C
	if (I2Cx == I2C1) {
		RCC->APB1ENR1	|= RCC_APB1ENR1_I2C1EN;		  // I2C2 clock enable
		RCC->CCIPR &= ~RCC_CCIPR_I2C1SEL;         // 00 = PCLK, 01 = SYSCLK, 10 = HSI16, 11 = Reserved
		RCC->CCIPR |=  RCC_CCIPR_I2C1SEL_0;       // Set I2C1 clock source as SYSCLK
	} else if (I2Cx == I2C2) {
		RCC->APB1ENR1	|= RCC_APB1ENR1_I2C2EN;	    // I2C3 clock enable
		RCC->CCIPR &= ~RCC_CCIPR_I2C2SEL;         // 00 = PCLK, 01 = SYSCLK, 10 = HSI16, 11 = Reserved
		RCC->CCIPR |=  RCC_CCIPR_I2C2SEL_0;       // Set I2C2 clock source as SYSCLK
	} else if (I2Cx == I2C3) {
		RCC->APB1ENR1	|= RCC_APB1ENR1_I2C3EN;    	// I2C3 clock enable
		RCC->CCIPR &= ~RCC_CCIPR_I2C3SEL;         // 00 = PCLK, 01 = SYSCLK, 10 = HSI16, 11 = Reserved
		RCC->CCIPR |=  RCC_CCIPR_I2C3SEL_0;       // Set I2C3 clock source as SYSCLK
	}

  /*---------------------------- I2Cx CR1 Configuration ----------------------*/
	// When the I2C is disabled (PE=0), the I2C performs a software reset.
	I2Cx->CR1 &= ~I2C_CR1_PE;       // Diable I2C


	/*---------------------------- I2Cx TIMINGR Configuration ------------------*/
	I2Cx->TIMINGR = TimingConfig;

	I2Cx->CR1 |= I2C_CR1_PE;								// Enable I2C1
}


int8_t I2C_Start(I2C_TypeDef * I2Cx, uint32_t DevAddress, uint8_t Size, uint8_t Direction)
{

	// Direction = 0: Master requests a write transfer
	// Direction = 1: Master requests a read transfer

	I2Cx->CR2 &= ~I2C_CR2_SADD;
	I2Cx->CR2 &= ~I2C_CR2_NBYTES;
	I2Cx->CR2 &= ~I2C_CR2_RELOAD;
	I2Cx->CR2 &= ~I2C_CR2_ADD10;
	I2Cx->CR2 |= I2C_CR2_AUTOEND;
	I2Cx->CR2 &= ~I2C_CR2_START;
	I2Cx->CR2 &= ~I2C_CR2_STOP;

	if (Direction == READ_FROM_PERIPHERAL)
		I2Cx->CR2 |= I2C_CR2_RD_WRN;  // Read from Peripheral
	if (Direction == WRITE_TO_PERIPHERAL)
		I2Cx->CR2 &= ~I2C_CR2_RD_WRN; // Write to Peripheral

  I2Cx->CR2 |= DevAddress<<1 & I2C_CR2_SADD;
	I2Cx->CR2 |= ((uint32_t)Size << 16 ) & I2C_CR2_NBYTES;

	I2Cx->CR2 |= I2C_CR2_START;


	if ( (I2Cx->ISR & I2C_ISR_NACKF) == I2C_ISR_NACKF )
		return -1;  // NACK was received; a communication error may have ocurred

   	return 0;  // Success
}


//===============================================================================
//                           Wait for the bus is ready
//===============================================================================
void I2C_WaitLineIdle(I2C_TypeDef * I2Cx)
{
	// Wait until I2C bus is ready
	while( (I2Cx->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY );	// If busy, wait
}


int8_t I2C_SendData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size)
{
	int i;

	if (Size <= 0 || pData == NULL)
		return -1;

	I2C_WaitLineIdle(I2Cx);

	if (I2C_Start(I2Cx, DeviceAddress, Size, WRITE_TO_PERIPHERAL) < 0 )
		return -1; // The I2C Start process was not successful

	for (i = 0; i < Size; i++)
	{
		// TXIS bit is set by hardware when the I2C_TXDR register is empty and the data to be
		// transmitted must be written in the I2C_TXDR register. It is cleared when the next data to be
		// sent is written in the I2C_TXDR register.
		// The TXIS flag is not set when a NACK is received.
		while((I2Cx->ISR & I2C_ISR_TXIS) == 0 );
		I2Cx->TXDR = pData[i] & I2C_TXDR_TXDATA;  // TXE is cleared by writing to the TXDR register.
	}

	// Wait until TC flag is set
	//while((I2Cx->ISR & I2C_ISR_TC) == 0 && (I2Cx->ISR & I2C_ISR_NACKF) == 0);

	if ( (I2Cx->ISR & I2C_ISR_NACKF) != 0 )
		return -1;

	return 0;
}

int8_t I2C_ReceiveData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size)
{
	int i;

	if (Size <= 0 || pData == NULL)
		return -1;

	I2C_WaitLineIdle(I2Cx);

	I2C_Start(I2Cx, DeviceAddress, Size, READ_FROM_PERIPHERAL); // 0 = sending data to the slave, 1 = receiving data from the slave

	for (i = 0; i < Size; i++)
	{
		// Wait until RXNE flag is set
		while( (I2Cx->ISR & I2C_ISR_RXNE) == 0 );
		pData[i] = I2Cx->RXDR & I2C_RXDR_RXDATA;
	}

	// Wait until TCR flag is set
	//while((I2Cx->ISR & I2C_ISR_TC) == 0);

	//I2C_Stop(I2Cx);

	return 0;
}

//===============================================================================
//                           VEML7700
//===============================================================================


/*
 * Write 16-bit value to a VEML7700 register.
 * VEML7700 expects:
 * Byte 0: register address
 * Byte 1: low byte
 * Byte 2: high byte
 */

int8_t VEML7700_WriteReg16(uint8_t reg, uint16_t value)
{
    uint8_t txData[3];

    txData[0] = reg;
    txData[1] = value & 0xFF;          // low byte first
    txData[2] = (value >> 8) & 0xFF;   // high byte second

    // Clear stale I2C flags before transaction
    I2C1->ICR |= I2C_ICR_NACKCF | I2C_ICR_STOPCF;

    return I2C_SendData(I2C1, VEML7700_ADDR_7BIT, txData, 3);
}

/*
 * Read 16-bit value from a VEML7700 register.
 * VEML7700 returns low byte first.
 */
uint16_t VEML7700_ReadReg16(uint8_t reg)
{
    uint8_t rxData[2] = {0, 0};
    uint16_t value = 0;

    // Clear stale I2C flags
    I2C1->ICR |= I2C_ICR_NACKCF | I2C_ICR_STOPCF;

    I2C_WaitLineIdle(I2C1);

    /*
     * Tell the sensor which register to read without generating STOP.
     * The VEML7700 read transaction needs a repeated START between the
     * register command byte and the 2-byte read phase.
     */
    I2C1->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD |
                   I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_ADD10 |
                   I2C_CR2_START | I2C_CR2_STOP);
    I2C1->CR2 |= ((uint32_t)VEML7700_ADDR_7BIT << 1) & I2C_CR2_SADD;
    I2C1->CR2 |= (1U << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES;
    I2C1->CR2 |= I2C_CR2_START;

    while ((I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)) == 0);
    if ((I2C1->ISR & I2C_ISR_NACKF) != 0) {
        return 0xFFFF;   // error marker
    }
    I2C1->TXDR = reg & I2C_TXDR_TXDATA;

    while ((I2C1->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)) == 0);
    if ((I2C1->ISR & I2C_ISR_NACKF) != 0) {
        return 0xFFFF;   // error marker
    }

    I2C1->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD |
                   I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_ADD10 |
                   I2C_CR2_START | I2C_CR2_STOP);
    I2C1->CR2 |= ((uint32_t)VEML7700_ADDR_7BIT << 1) & I2C_CR2_SADD;
    I2C1->CR2 |= (2U << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES;
    I2C1->CR2 |= I2C_CR2_RD_WRN | I2C_CR2_AUTOEND | I2C_CR2_START;

    for (int i = 0; i < 2; i++) {
        while ((I2C1->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF)) == 0);
        if ((I2C1->ISR & I2C_ISR_NACKF) != 0) {
            return 0xFFFF;   // error marker
        }
        rxData[i] = I2C1->RXDR & I2C_RXDR_RXDATA;
    }

    while ((I2C1->ISR & I2C_ISR_STOPF) == 0);
    I2C1->ICR |= I2C_ICR_STOPCF;

    if ((I2C1->ISR & I2C_ISR_NACKF) != 0) {
        return 0xFFFF;   // error marker
    }

    // VEML7700 sends low byte first
    value = ((uint16_t)rxData[1] << 8) | rxData[0];

    return value;
}

// If need to check status - Add these to main.c
//
// uint16_t config_readback = 0;
// VEML7700_WriteReg16(VEML7700_ALS_CONF_0, 0x0000);

