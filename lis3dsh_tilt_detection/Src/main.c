/*
 * main.c
 *
 *  Created on: Sep 25, 2021
 *      Author: vish
 */

/*
 * PA5 = SPI1_SCK
 * PA6 = SPI1_MISO
 * PA7 = SPI1_MOSI
 * PE3 = cs
 *
 * PE0 = MEMS_INT1
 * PE1 = MEMS_INT2
 */
/*
 * PA7 = SPI1_MOSI = ch0
 * PA6 = SPI1_MISO = ch1
 * PA5 = SPI1_SCK  = ch2
 * PE3 = cs enable = ch3
 */

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>

//SPI1->SR bits
#define SPI1_RXNE		(1<<0)		// RXNE: Receive buffer not empty
#define SPI1_TXE		(1<<1)		// TXE: Transmit buffer empty
#define SPI1_BSY		(1<<7)		// BSY: Busy flag

#define READ_OP 		(1<<7)
#define WHO_AM_I 		(0x0F) // r OF 0011 1111 Who I am ID

//LIS3DSH Control Registers
#define CTRL_REG4		(0x20)
#define CTRL_REG1		(0x21)
#define CTRL_REG2		(0x22)
#define CTRL_REG3 		(0x23)
#define CTRL_REG5		(0x24)
#define CTRL_REG6 		(0x25)

#define TIM1_1L		(0x55)
#define TIM2_1L		(0x52)
#define TIM3_1			(0x51)
#define TIM4_1			(0x50)

#define THRS2_1		(0x56)
#define THRS1_1		(0x57)

#define MASK1_B		(0x59)
#define MASK1_A		(0x5A)

#define SETT1			(0x5B)
#define ST1_1			(0x40)
#define ST1_2			(0x41)
#define ST1_3			(0x42)
#define ST1_4			(0x43)
#define ST1_5			(0x44)
#define ST1_6			(0x45)
#define ST1_7			(0x46)
#define ST1_8			(0x47)
#define ST1_9			(0x48)
#define ST1_10			(0x49)
#define ST1_11			(0x4A)

#define STATUS_REG		(0x27)
#define XOUT_L_REG		(0x28)
#define XOUT_H_REG		(0x29)
#define YOUT_L_REG		(0x2A)
#define YOUT_H_REG		(0x2B)
#define ZOUT_L_REG		(0x2C)
#define ZOUT_H_REG		(0x2D)

#define SENSITYVITY_FS2	(0.06)
#define MULTIPLEBYTE_CMD 	(0x40)
#define abs(x)			( x<0 ? -(x) : x )  //remove negative sign
#define ThresholdHigh		(350)
#define ThresholdLow		(-350)

/************** SPI_config STEPS TO FOLLOW *****************
1. Enable SPI clock
2. Configure the Control Register 1
3. Configure the CR2
************************************************/
void SPI_Config (void)
{
	RCC->APB2ENR |= (1<<12); //enable SPI1 clock

	SPI1->CR1 |= (1<<0);	// cpha = 1, sample data at second edge (rising edge because cpol=1)

	SPI1->CR1 |= (1<<1); 	// cpol = 1, idle_clock is high

	SPI1->CR1 |= (1<<2); 	// Master ModeLIS3DSH_write(TIM1_1L,0x07); //Freefall duration (= 25 ms)

	SPI1->CR1 &= ~(7<<3); 	// BR[2:0] = 000; default_clock/2 = 8MHz

	SPI1->CR1 &= ~(1<<7); 	// LSBFIRST = 0, set for MSB first

	SPI1->CR1 |= (1<<8); 	// SSI: Internal slave select

	SPI1->CR1 |= (1<<9);	//SSM: Software slave management

	SPI1->CR1 &= ~(1<<10);  // RXONLY = 0, full-duplex

	SPI1->CR1 &= ~(1<<11);  // DFF=0, 8 bit data

	SPI1->CR2 = 0;

	SPI1->CR1 |= (1<<6); //Enable SPI module

}

/************** SPI_transmit STEPS TO FOLLOW *****************
1. Wait for the TXE bit to set in the Status Register
2. Write the data to the Data Register
3. After the data has been transmitted, wait for the BSY bit to reset in Status Register
4. Clear the Overrun flag by reading DR and SR
************************************************/
void SPI_transmit (uint8_t data)
{

	while(!((SPI1->SR) & SPI1_TXE)); //wait for TXE bit to set

	SPI1->DR = data;

	while(!((SPI1->SR) & SPI1_TXE)); // wait for TXE bit to set (until buffer empty)

	while (((SPI1->SR) & SPI1_BSY)); // wait for BSY bit to Reset (until SPI not busy)

	//clear the overrun flag by reading DR and SR
	uint16_t temp;
	temp = SPI1->DR;
	temp = SPI1->SR;
	temp++; //just to avoid unused temp variable warning
}

/************** SPI1_receive STEPS TO FOLLOW *****************
	1. Wait for the BSY bit to reset in Status Register
	2. Send some Dummy data before reading the DATA
	3. Wait for the RXNE bit to Set in the status Register
	4. Read data from Data Register
	************************************************/

void SPI_receive (int8_t *data)
{

		while (((SPI1->SR) & SPI1_BSY)); // wait for BSY bit to Reset (until SPI not busy)

		SPI1->DR = 0; //send dummy data

		while (!((SPI1->SR)& SPI1_RXNE));	// Wait for RXNE to set

		*data = (SPI1->DR);
}

void GPIO_Config (void)
{
	RCC->AHB1ENR |= (1<<0) | (1<<3) | (1<<4); //enable GPIO port A, D & port E

	GPIOA->MODER |= (2<<10) | (2<<12) | (2<<14); //PA5, PA6, & PA7 set for alternate function (SPI pins)
	GPIOD->MODER |= (1<<24) | (1<<26) | (1<<28) | (1<<30);	// PD12/13/14/15 set for output (LED pin)
	GPIOE->MODER |= (1<<6);		// PE3 set for output (slave select pin)
	GPIOE->MODER &= ~(1<<0);	// PE0 & PE1 set for interrupt input

	//port A
	GPIOA->OSPEEDR |= (3<<10)|(3<<12)|(3<<14);  // HIGH Speed for PA5, PA6, PA7
	GPIOA->AFR[0] |= (5<<20)|(5<<24)|(5<<28);   // AF5(SPI1) for PA5, PA6, PA7

	//port D
	GPIOD->OTYPER &= ~((1<<13) | (1<<13) | (1<<14) | (1<<13));  // bit = 13 --> Output push pull
	GPIOD->OSPEEDR |= (2<<24) | (2<<26) | (2<<28) | (2<<30);  // Pin PD13 (bits 27:26) as Fast Speed (1:0)
	GPIOA->PUPDR &= ~((3<<24) | (3<<26) | (3<<28) | (3<<30));  // Pin PD13 (bits 11:10) are 1:0 --> no pull up or pulldown

	//port E
	GPIOE->PUPDR |=  (2<<0) | (2<<2);  // Bits (1:0) = 1:0  --> PE0 is in Pull Down mode
	GPIOE->ODR |= (1<<3); // PE3 pin set, put slave idle
}

/*************>>>>>>> EXTI Interrupt STEPS FOLLOWED <<<<<<<<************

1. Enable the SYSCFG/AFIO bit in RCC register
2. Configure the EXTI configuration Register in the SYSCFG/AFIO
3. Disable the EXTI Mask using Interrupt Mask Register (IMR)
4. Configure the Rising Edge / Falling Edge Trigger
5. Set the Interrupt Priority
6. Enable the interrupt

********************************************************/
void exti_init(void)
{
	/* Disable global interrupts */
	__disable_irq();

	RCC->APB2ENR |= (1<<14);//Enable SYSCNFG

	SYSCFG->EXTICR[0] |= (0x4<<0) | (0x4<<4);	// configure EXTI0 line for PE0 & EXTI1 line for PE1

	EXTI->IMR |= (1<<0) | (1<<1);  // Disable the Mask on EXTI 0 & EXTI 1

	EXTI->RTSR &= ~((1<<0) | (1<<1));  // Disable Rising Edge Trigger for PE0 & PE1

	EXTI->FTSR |= (1<<0) | (1<<1);  // Enable Falling Edge Trigger for PE0 & PE1

	NVIC_SetPriority (EXTI0_IRQn, 0); // Set Priority for PE0
	NVIC_SetPriority (EXTI1_IRQn, 0); // Set Priority for PE1

	NVIC_EnableIRQ (EXTI0_IRQn);  // Enable Interrupt for PE0
	NVIC_EnableIRQ (EXTI1_IRQn);  // Enable Interrupt for PE1

	__enable_irq(); /* Enable global interrupts */
}

void EXTI0_IRQHandler(void)
{
	if (EXTI->PR & (1<<0))    // If the PE0 triggered the interrupt
	{
		GPIOD->ODR |=  (1<<12) | (1<<13) | (1<<14) | (1<<15) ; //turn all led ON

		EXTI->PR |= (1<<0);  // Clear the interrupt flag by writing a 1
	}

}

void EXTI1_IRQHandler(void)
{
	if (EXTI->PR & (1<<1))    // If the PE1 triggered the interrupt
	{
		GPIOD->ODR |= (1<<14); // Set the Pin PD14 : red led

		EXTI->PR |= (1<<1);  // Clear the interrupt flag by writing a 1
	}

}

/*LIS3DSH CS pin other than slave select
 * also work as mode select pin
 * to select between I2C and SPI mode
 * CS=0 for SPI, CS=1 for I2C
 */
void CS_Enable (void)
{
	GPIOE->ODR &= ~(1<<3); // PE3 pin reset, slave selected

}

void CS_Disable (void)
{
	GPIOE->ODR |= (1<<3); // PE3 pin set, put slave idle
}

void LIS3DSH_write (uint8_t address, uint8_t value)
{
	CS_Enable();//pull cs pin low

	SPI_transmit(address); // select register
	SPI_transmit(value); // write data on register

	CS_Disable();	//pull the pin high
}

void LIS3DSH_read (uint8_t address, int8_t* Data )
{
	address |= READ_OP ; //read operation bit set and multiple byte IO bit set

	CS_Enable();	// pull pin low, set slave in SPI mode

	SPI_transmit (address); //send address
	SPI_receive (Data); // receive raw data respective to size in bytes

	CS_Disable(); //pull the pin high

}


void LIS3DSH_init(void)
{
	/* Set configuration of LIS3DSH MEMS Accelerometer */
	LIS3DSH_write(CTRL_REG4,0x67); // ODR = 100 Hz; x = y = z = enable;
	LIS3DSH_write(CTRL_REG5,0x00); // SPI = 4 wire; self_test = noramal; full_scale = +-2g; Anti-Aliasing_Filter_Bandwidth = 800;

}



int main()
{
	int ax, ay, az;
	int8_t RxData[6] = {0};

	GPIO_Config();
	SPI_Config();
	//exti_init();
	
	LIS3DSH_init();
	


	LIS3DSH_read(WHO_AM_I, RxData); //read who_am_I register to confirm selected slave


	while(1)
	{

		LIS3DSH_read(XOUT_L_REG, &RxData[0]);
		LIS3DSH_read(XOUT_H_REG, &RxData[1]);
		LIS3DSH_read(YOUT_L_REG, &RxData[2]);
		LIS3DSH_read(YOUT_H_REG, &RxData[3]);
		LIS3DSH_read(ZOUT_L_REG, &RxData[4]);
		LIS3DSH_read(ZOUT_H_REG, &RxData[5]);

		// process data
		ax = (SENSITYVITY_FS2 * ( (RxData[1]<<8) + (RxData[0]) ));
		ay = (SENSITYVITY_FS2 * ( (RxData[3]<<8) + (RxData[2]) ));
		az = (SENSITYVITY_FS2 * ( (RxData[5]<<8) + (RxData[4]) ));


		/* process for tilt led code */
		if(abs(ax) > abs(ay) )
		{
			if(ax > ThresholdHigh)
				GPIOD->ODR |= (1<<12); //led4 : green : left

			else if(ax < ThresholdLow)
				GPIOD->ODR |= (1<<14); //led5 : red : right

		}

		else
		{
			if(ay > ThresholdHigh)
				GPIOD->ODR |= (1<<15); //led6 : blue : down

			else if(ay < ThresholdLow)
				GPIOD->ODR |= (1<<13); //led3 : orange : up
		}

		for(uint32_t i = 1; i == 0; i++);

		GPIOD->ODR &= ~( (1<<12) | (1<<13) | (1<<14) | (1<<15) ); //turn all led off

	}

	return 0;
}
