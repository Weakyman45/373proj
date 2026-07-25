################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Startup/Src/LCD.c \
../Core/Startup/Src/main.c \
../Core/Startup/Src/stm32f1xx_hal_msp.c \
../Core/Startup/Src/stm32f1xx_it.c \
../Core/Startup/Src/syscalls.c \
../Core/Startup/Src/sysmem.c \
../Core/Startup/Src/system_stm32f1xx.c 

OBJS += \
./Core/Startup/Src/LCD.o \
./Core/Startup/Src/main.o \
./Core/Startup/Src/stm32f1xx_hal_msp.o \
./Core/Startup/Src/stm32f1xx_it.o \
./Core/Startup/Src/syscalls.o \
./Core/Startup/Src/sysmem.o \
./Core/Startup/Src/system_stm32f1xx.o 

C_DEPS += \
./Core/Startup/Src/LCD.d \
./Core/Startup/Src/main.d \
./Core/Startup/Src/stm32f1xx_hal_msp.d \
./Core/Startup/Src/stm32f1xx_it.d \
./Core/Startup/Src/syscalls.d \
./Core/Startup/Src/sysmem.d \
./Core/Startup/Src/system_stm32f1xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/Src/%.o Core/Startup/Src/%.su Core/Startup/Src/%.cyclo: ../Core/Startup/Src/%.c Core/Startup/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Startup-2f-Src

clean-Core-2f-Startup-2f-Src:
	-$(RM) ./Core/Startup/Src/LCD.cyclo ./Core/Startup/Src/LCD.d ./Core/Startup/Src/LCD.o ./Core/Startup/Src/LCD.su ./Core/Startup/Src/main.cyclo ./Core/Startup/Src/main.d ./Core/Startup/Src/main.o ./Core/Startup/Src/main.su ./Core/Startup/Src/stm32f1xx_hal_msp.cyclo ./Core/Startup/Src/stm32f1xx_hal_msp.d ./Core/Startup/Src/stm32f1xx_hal_msp.o ./Core/Startup/Src/stm32f1xx_hal_msp.su ./Core/Startup/Src/stm32f1xx_it.cyclo ./Core/Startup/Src/stm32f1xx_it.d ./Core/Startup/Src/stm32f1xx_it.o ./Core/Startup/Src/stm32f1xx_it.su ./Core/Startup/Src/syscalls.cyclo ./Core/Startup/Src/syscalls.d ./Core/Startup/Src/syscalls.o ./Core/Startup/Src/syscalls.su ./Core/Startup/Src/sysmem.cyclo ./Core/Startup/Src/sysmem.d ./Core/Startup/Src/sysmem.o ./Core/Startup/Src/sysmem.su ./Core/Startup/Src/system_stm32f1xx.cyclo ./Core/Startup/Src/system_stm32f1xx.d ./Core/Startup/Src/system_stm32f1xx.o ./Core/Startup/Src/system_stm32f1xx.su

.PHONY: clean-Core-2f-Startup-2f-Src

