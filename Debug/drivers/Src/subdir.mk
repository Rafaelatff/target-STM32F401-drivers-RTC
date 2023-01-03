################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/Src/stm32f401xx.c \
../drivers/Src/stm32f401xx_gpio_driver.c \
../drivers/Src/stm32f401xx_i2c_driver.c \
../drivers/Src/stm32f401xx_spi_driver.c 

OBJS += \
./drivers/Src/stm32f401xx.o \
./drivers/Src/stm32f401xx_gpio_driver.o \
./drivers/Src/stm32f401xx_i2c_driver.o \
./drivers/Src/stm32f401xx_spi_driver.o 

C_DEPS += \
./drivers/Src/stm32f401xx.d \
./drivers/Src/stm32f401xx_gpio_driver.d \
./drivers/Src/stm32f401xx_i2c_driver.d \
./drivers/Src/stm32f401xx_spi_driver.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/Src/%.o drivers/Src/%.su: ../drivers/Src/%.c drivers/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F401RE -DSTM32 -DSTM32F401RETx -DSTM32F4 -c -I../Inc -I"C:/Users/r.freitas/Documents/Rafaela/Cursos/Microcontroller Embedded C Programming - Absolute Beginners/Embedded-C/My_workspace/target/target-STM32F401-drivers-I2C-IT/drivers/Inc" -I"C:/Users/r.freitas/Documents/Rafaela/Cursos/Microcontroller Embedded C Programming - Absolute Beginners/Embedded-C/My_workspace/target/target-STM32F401-drivers-I2C-IT/bsp" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-drivers-2f-Src

clean-drivers-2f-Src:
	-$(RM) ./drivers/Src/stm32f401xx.d ./drivers/Src/stm32f401xx.o ./drivers/Src/stm32f401xx.su ./drivers/Src/stm32f401xx_gpio_driver.d ./drivers/Src/stm32f401xx_gpio_driver.o ./drivers/Src/stm32f401xx_gpio_driver.su ./drivers/Src/stm32f401xx_i2c_driver.d ./drivers/Src/stm32f401xx_i2c_driver.o ./drivers/Src/stm32f401xx_i2c_driver.su ./drivers/Src/stm32f401xx_spi_driver.d ./drivers/Src/stm32f401xx_spi_driver.o ./drivers/Src/stm32f401xx_spi_driver.su

.PHONY: clean-drivers-2f-Src

