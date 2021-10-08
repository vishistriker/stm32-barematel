################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Inc/Delay.c \
../Inc/RccConfig.c \
../Inc/mpu6050.c \
../Inc/twi.c 

OBJS += \
./Inc/Delay.o \
./Inc/RccConfig.o \
./Inc/mpu6050.o \
./Inc/twi.o 

C_DEPS += \
./Inc/Delay.d \
./Inc/RccConfig.d \
./Inc/mpu6050.d \
./Inc/twi.d 


# Each subdirectory must supply rules for building sources it contributes
Inc/%.o: ../Inc/%.c Inc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -DSTM32F407xx -c -I../Inc -I/opt/st/stm32cubeide_1.7.0/Chipheaders/Device/ST/STM32F4xx/Include -I/opt/st/stm32cubeide_1.7.0/Chipheaders/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

