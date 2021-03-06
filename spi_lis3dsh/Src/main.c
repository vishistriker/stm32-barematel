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
 * PE3 =
 */

#include <stm32f4xx.h>
#include <stdint.h>

//SPI1->SR bits
#define SPI1_RXNE		(1<<0)		// RXNE: Receive buffer not empty
#define SPI1_TXE		(1<<1)		// TXE: Transmit buffer empty
#define SPI1_BSY		(1<<7)		// BSY: Busy flag

#define SENSITYVITY_FS2	(0.06)

float xg, yg, zg;
int16_t x,y,z;
uint8_t RxData[6];

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

	SPI1->CR1 |= (1<<2); 	// Master Mode

	SPI1->CR1 |= (1<<3); 	// BR[2:0] = 011; SSM=1, SSI=1 ->software slave management

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
void SPI_transmit (uint8_t *data, int size)
{
	for(uint16_t i=0; i<size; i++)
	{
		while(!((SPI1->SR) & SPI1_TXE)); //wait for TXE bit to set

		SPI1->DR = data[i]; // load the data into the data
	}

	while(!((SPI1->SR) & SPI1_TXE)); // wait for TXE bit to set (until buffer empty)

	while (((SPI1->SR) & SPI1_BSY)); // wait for BSY bit to Reset (until SPI not busy)

	//clear the overrun flag by reading DR and SR
	uint16_t temp;
	temp = SPI1->DR;
	temp = SPI1->SR;
}

/************** SPI1_receive STEPS TO FOLLOW *****************
	1. Wait for the BSY bit to reset in Status Register
	2. Send some Dummy data before reading the DATA
	3. Wait for the RXNE bit to Set in the status Register
	4. Read data from Data Register
	************************************************/

void SPI_receive (uint8_t *data, int size)
{
	for(int i=0; size>0; size--,i++)
	{
		while (((SPI1->SR) & SPI1_BSY)); // wait for BSY bit to Reset (until SPI not busy)

		SPI1->DR = 0; //send dummy data

		while (!((SPI1->SR)& SPI1_RXNE));	// Wait for RXNE to set

		data[i] = (SPI1->DR);
	}
}

void GPIO_Config (void)
{
	RCC->AHB1ENR |= (1<<0) | (1<<4); //enable GPIO port A & port E

	GPIOA->MODER |= (2<<10) | (2<<12) | (2<<14); //PA5, PA6, & PA7 set for alternate function
	GPIOE->MODER |= (1<<6);	// PE3 set for output

	GPIOA->OSPEEDR |= (3<<10)|(3<<12)|(3<<14);  // HIGH Speed for PA5, PA6, PA7, PA9

	GPIOA->AFR[0] |= (5<<20)|(5<<24)|(5<<28);   // AF5(SPI1) for PA5, PA6, PA7

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
	uint8_t data[2];
	data[0]= address;// | (1<<6);  // multiple write
	data[1]=value;

	CS_Enable();//pull cs pin low
	SPI_transmit(data, 2); // write data to register
	CS_Disable();	//pull the pin high
}

void LIS3DSH_read (uint8_t address)
{
	address |= (1<<7); //MSB=1 for Read operation

	CS_Enable();	// pull pin low, set slave in SPI mode

	SPI_transmit (&address, 1);//send address
	SPI_receive (RxData, 1); // receive 6 bytes data
	CS_Disable(); //pull the pin high

}

void LIS3DSH_init(void)
{
	GPIO_Config();
	SPI_Config();

	LIS3DSH_read(0x0F); //read who_am_I register to confirm selected slave

}

int main(){

	int16_t ax, ay, az;
	
	LIS3DSH_init();

	while(1)
	{
		LIS3DSH_read(0x28);
		
		// process data
		ax = (SENSITYVITY_FS2 * ( (RxData[1]<<8) + (RxData[0]) ));
		ay = (SENSITYVITY_FS2 * ( (RxData[3]<<8) + (RxData[2]) ));
		az = (SENSITYVITY_FS2 * ( (RxData[5]<<8) + (RxData[4]) ));
	}

	return 0;
}
