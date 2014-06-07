
# The name of your project (used to name the compiled .hex file)
TARGET = main

# configurable options
OPTIONS = -DF_CPU=48000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH

# options needed by many Arduino libraries to configure for Teensy 3.0
OPTIONS += -D__MK20DX128__ -DARDUIO=104


#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

# path location for Teensy Loader, teensy_post_compile and teensy_reboot
ARDUINOPATH = /Applications/Arduino.app/Contents/Resources/Java/
TOOLSPATH = $(ARDUINOPATH)/hardware/tools/avr/bin   # on Linux
TOOLSPATH = $(ARDUINOPATH)/hardware/tools	# on Linux
#TOOLSPATH = ../../../tools   # on Linux
#TOOLSPATH = ../../../tools/avr/bin   # on Mac or Windows

# path location for Arduino libraries (currently not used)
#LIBRARYPATH = ../../../../libraries
LIBRARYPATH = $(ARDUINOPATH)/libraries

# path location for the arm-none-eabi compiler
#COMPILERPATH = ../../../tools/arm-none-eabi/bin
COMPILERPATH = $(ARDUINOPATH)/hardware/tools/arm-none-eabi/bin

#TEENSY3 = $(ARDUINOPATH)/hardware/teensy/cores/teensy3
TEENSY3 = teensy3
LDFILE = teensy3/mk20dx256.ld

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -g -Os -mcpu=cortex-m4 -mthumb -nostdlib -MMD $(OPTIONS) -I. -I$(TEENSY3)

# compiler options for C++ only
CXXFLAGS = -std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti

# compiler options for C only
CFLAGS =

# linker options
LDFLAGS = -Os -Wl,--gc-sections -mcpu=cortex-m4 -mthumb -T$(LDFILE) -Lteensy3

# additional libraries to link
LIBS = -lm -lmain


# names for the compiler programs
CC = $(abspath $(COMPILERPATH))/arm-none-eabi-gcc
CXX = $(abspath $(COMPILERPATH))/arm-none-eabi-g++
OBJCOPY = $(abspath $(COMPILERPATH))/arm-none-eabi-objcopy
SIZE = $(abspath $(COMPILERPATH))/arm-none-eabi-size
AR = $(abspath $(COMPILERPATH))/arm-none-eabi-ar
RANLIB = $(abspath $(COMPILERPATH))/arm-none-eabi-ranlib

# automatically create lists of the sources and objects
# TODO: this does not handle Arduino libraries yet...
C_FILES := $(wildcard *.c)
CPP_FILES := $(wildcard *.cpp)
OBJS := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)


# the actual makefile rules (all .o files built by GNU make's default implicit rules)

all: $(TARGET).hex

lib: lib$(TARGET).a 

lib$(TARGET).a: $(OBJS) Makefile
	$(AR) cvr $@ $(OBJS)
	$(RANLIB) $@

$(TARGET).elf: $(OBJS) $(LDFILE) Makefile
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.hex: %.elf
	$(SIZE) $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(abspath $(TOOLSPATH))/teensy_post_compile -file=$(basename $@) -path=$(shell pwd) -tools=$(abspath $(TOOLSPATH))
	-$(abspath $(TOOLSPATH))/teensy_reboot


# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	rm -f *.o *.d $(TARGET).elf $(TARGET).hex *.a


