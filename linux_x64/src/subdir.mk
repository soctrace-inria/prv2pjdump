################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/paraverParser.cpp \
../src/prv2pjdump.cpp \
../src/event.cpp 

OBJS += \
./src/paraverParser.o \
./src/prv2pjdump.o \
./src/event.o

CPP_DEPS += \
./src/paraverParser.d \
./src/prv2pjdump.d \
./src/event.d

# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0  -std=c++11 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


