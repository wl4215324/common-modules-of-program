################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/net_connection/tcp_client.c 

OBJS += \
./Src/net_connection/tcp_client.o 

C_DEPS += \
./Src/net_connection/tcp_client.d 


# Each subdirectory must supply rules for building sources it contributes
Src/net_connection/%.o: ../Src/net_connection/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


