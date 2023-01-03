################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/010rtc-lcd.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/010rtc-lcd.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/010rtc-lcd.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F401RE -DSTM32 -DSTM32F401RETx -DSTM32F4 -c -I../Inc -I"C:/Users/r.freitas/Documents/Rafaela/Cursos/Microcontroller Embedded C Programming - Absolute Beginners/Embedded-C/My_workspace/target/target-STM32F401-drivers-I2C-IT/drivers/Inc" -I"C:/Users/r.freitas/Documents/Rafaela/Cursos/Microcontroller Embedded C Programming - Absolute Beginners/Embedded-C/My_workspace/target/target-STM32F401-drivers-I2C-IT/bsp" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/010rtc-lcd.d ./Src/010rtc-lcd.o ./Src/010rtc-lcd.su ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

