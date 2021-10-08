/*
 * mpu6050.c
 *
 *  Created on: Sep 20, 2021
 *      Author: vish
 */

#include <stdint.h>
#include "mpu6050.h"

void MPU_Write (uint8_t Address, uint8_t Reg, uint8_t Data)
{
	I2C_Start ();
	I2C_Address (Address);
	I2C_Write (Reg);
	I2C_Write (Data);
	I2C_Stop ();
}

void MPU_Read (uint8_t Address, uint8_t Reg, uint8_t *buffer, uint8_t size)
{
	I2C_Start ();
	I2C_Address (Address);
	I2C_Write (Reg);
	I2C_Start ();  // repeated start
	I2C_Read (Address+0x01, buffer, size);
	I2C_Stop ();
}


void MPU6050_Init (void)
{
	uint8_t check;
	uint8_t Data;

	// check device ID WHO_AM_I

	MPU_Read (MPU6050_ADDR,WHO_AM_I_REG, &check, 1);

	if (check == 104)  // 0x68 will be returned by the sensor if everything goes well
	{
		// power management register 0X6B we should write all 0's to wake the sensor up
		Data = 0;
		MPU_Write (MPU6050_ADDR, PWR_MGMT_1_REG, Data);

		// Set DATA RATE of 1KHz by writing SMPLRT_DIV register
		Data = 0x07;
		MPU_Write(MPU6050_ADDR, SMPLRT_DIV_REG, Data);

		// Set accelerometer configuration in ACCEL_CONFIG Register
		// XA_ST=0,YA_ST=0,ZA_ST=0, FS_SEL=0 -> ? 2g
		Data = 0x00;
		MPU_Write(MPU6050_ADDR, ACCEL_CONFIG_REG, Data);

		// Set Gyroscope configuration in GYRO_CONFIG Register
		// XG_ST=0,YG_ST=0,ZG_ST=0, FS_SEL=0 -> ? 250 ?/s
		Data = 0x00;
		MPU_Write(MPU6050_ADDR, GYRO_CONFIG_REG, Data);
	}

}

void MPU6050_Read_Gyro (sensor_values * MPU6050)
{

	uint8_t Rx_data[6];

	// Read 6 BYTES of data starting from ACCEL_XOUT_H register

	MPU_Read (MPU6050_ADDR, GYRO_XOUT_H_REG, Rx_data, 6);

	MPU6050->Gyro_X_RAW = (int16_t)(Rx_data[0] << 8 | Rx_data [1]);
	MPU6050->Gyro_Y_RAW  = (int16_t)(Rx_data[2] << 8 | Rx_data [3]);
	MPU6050->Gyro_Z_RAW  = (int16_t)(Rx_data[4] << 8 | Rx_data [5]);

	/*** convert the RAW values into acceleration in 'g'
	     we have to divide according to the Full scale value set in FS_SEL
	     I have configured FS_SEL = 0. So I am dividing by 16384.0
	     for more details check ACCEL_CONFIG Register              ****/

	MPU6050->Gx = MPU6050->Gyro_X_RAW /16384.0;
	MPU6050->Gy = MPU6050->Gyro_Y_RAW /16384.0;
	MPU6050->Gz = MPU6050->Gyro_Z_RAW /16384.0;
}

void MPU6050_Read_Accel (sensor_values * MPU6050)
{

	uint8_t Rx_data[6];

	// Read 6 BYTES of data starting from ACCEL_XOUT_H register

	MPU_Read (MPU6050_ADDR, ACCEL_XOUT_H_REG, Rx_data, 6);

	MPU6050->Accel_X_RAW = (int16_t)(Rx_data[0] << 8 | Rx_data [1]);
	MPU6050->Accel_Y_RAW = (int16_t)(Rx_data[2] << 8 | Rx_data [3]);
	MPU6050->Accel_Z_RAW = (int16_t)(Rx_data[4] << 8 | Rx_data [5]);

	/*** convert the RAW values into acceleration in 'g'
	     we have to divide according to the Full scale value set in FS_SEL
	     I have configured FS_SEL = 0. So I am dividing by 16384.0
	     for more details check ACCEL_CONFIG Register              ****/

	MPU6050->Ax = MPU6050->Accel_X_RAW/16384.0;
	MPU6050->Ay = MPU6050->Accel_Y_RAW/16384.0;
	MPU6050->Az = MPU6050->Accel_Z_RAW/16384.0;
}


