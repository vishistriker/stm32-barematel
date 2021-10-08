

#include "RccConfig.h"
#include "Delay.h"
#include "twi.h"
#include "mpu6050.h"

sensor_values MPU6050;

int main ()
{

	//sensor_values MPU6050;

	SysClockConfig ();
	TIM6Config ();
	I2C_Config ();

	MPU6050_Init ();

	for(;;)
	{
		MPU6050_Read_Accel (&MPU6050);
		//MPU6050_Read_Gyro (&MPU6050);
		Delay_ms (1000);
	}

}
