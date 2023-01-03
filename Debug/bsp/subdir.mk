################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bsp/ds1307.c 

OBJS += \
./bsp/ds1307.o 

C_DEPS += \
./bsp/ds1307.d 


# Each subdirectory must supply rules for building sources it contributes
bsp/%.o bsp/%.su: ../bsp/%.c bsp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F401RE -DSTM32 -DSTM32F401RETx -DSTM32F4 -c -I../Inc -I"C:/Users/r.freitas/Documents/Rafaela/Cursos/Microcontroller Embedded C Programming - Absolute Beginners/Embedded-C/My_workspace/target/target-STM32F401-drivers-I2C-IT/drivers/Inc" -I"C:/Users/r.freitas/Documents/Rafaela/Cursos/Microcontroller Embedded C Programming - Absolute Beginners/Embedded-C/My_workspace/target/target-STM32F401-drivers-I2C-IT/bsp" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-bsp

clean-bsp:
	-$(RM) ./bsp/ds1307.d ./bsp/ds1307.o ./bsp/ds1307.su

.PHONY: clean-bsp

