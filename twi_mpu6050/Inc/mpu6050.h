/*
 * mpu6050.h
 *
 *  Created on: Sep 20, 2021
 *      Author: vish
 */

#ifndef MPU6050_H_
#define MPU6050_H_

#define MPU6050_ADDR 0xD0

#define SMPLRT_DIV_REG 0x19
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41
#define GYRO_XOUT_H_REG 0x43
#define PWR_MGMT_1_REG 0x6B
#define WHO_AM_I_REG 0x75

typedef struct{
	int16_t Accel_X_RAW ;
	int16_t Accel_Y_RAW ;
	int16_t Accel_Z_RAW ;

	int16_t Gyro_X_RAW ;
	int16_t Gyro_Y_RAW ;
	int16_t Gyro_Z_RAW ;

	float Ax, Ay, Az, Gx, Gy, Gz;
}sensor_values;

void MPU6050_Init (void);
void MPU_Write (uint8_t Address, uint8_t Reg, uint8_t Data);
void MPU_Read (uint8_t Address, uint8_t Reg, uint8_t *buffer, uint8_t size);
void MPU6050_Read_Accel (sensor_values * MPU6050);
void MPU6050_Read_Gyro (sensor_values * MPU6050);

#endif /* MPU6050_H_ */
