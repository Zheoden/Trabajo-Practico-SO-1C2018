################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Funciones/conexiones.c \
../Funciones/configs.c \
../Funciones/lista.c \
../Funciones/logs.c \
../Funciones/serializacion.c 

OBJS += \
./Funciones/conexiones.o \
./Funciones/configs.o \
./Funciones/lista.o \
./Funciones/logs.o \
./Funciones/serializacion.o 

C_DEPS += \
./Funciones/conexiones.d \
./Funciones/configs.d \
./Funciones/lista.d \
./Funciones/logs.d \
./Funciones/serializacion.d 


# Each subdirectory must supply rules for building sources it contributes
Funciones/%.o: ../Funciones/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


