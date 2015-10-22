################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/paraverParser.cpp \
../src/prv2pjdump.cpp 

OBJS += \
./src/paraverParser.o \
./src/prv2pjdump.o 

CPP_DEPS += \
./src/paraverParser.d \
./src/prv2pjdump.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	x86_64-w64-mingw32-g++ -O3 -Wall -c -fmessage-length=0  -std=c++11 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


