# The name of your project (used to name the compiled .hex file)
TARGET = $(notdir $(CURDIR))

# The teensy version to use, 30 or 31
TEENSY = 31

# Set to 24000000, 48000000, or 96000000 to set CPU core speed
TEENSY_CORE_SPEED = 96000000
#TEENSY_CORE_SPEED = 120000000

# Some libraries will require this to be defined
# If you define this, you will break the default main.cpp
#ARDUINO = 105

# configurable options
OPTIONS = -DUSB_SERIAL -DLAYOUT_US_ENGLISH -DARDUINO -DTEENSYDUINO=120

# directory to build in
BUILDDIR = $(abspath $(CURDIR)/build)

#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

# path location for Teensy Loader, teensy_post_compile and teensy_reboot
TOOLSPATH = $(CURDIR)/tools

ARDUINOPATH = /Applications/Arduino.app/Contents/Resources/Java

ifeq ($(OS),Windows_NT)
    $(error What is Win Dose?)
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        TOOLSPATH = $(ARDUINOPATH)/hardware/tools/
    endif
endif

# path location for Teensy 3 core
COREPATH = teensy3
COREPATH = $(ARDUINOPATH)/hardware/teensy/cores/teensy3

# path location for Arduino libraries
ARDUINOLIBRARYPATH = $(ARDUINOPATH)/libraries
LIBRARYPATH = libraries
ARDUINOLIBRARIES = Encoder SD SPI Wire

# path location for the arm-none-eabi compiler
COMPILERPATH = $(TOOLSPATH)/arm-none-eabi/bin

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -g -Os -mcpu=cortex-m4 -mthumb -nostdlib -MMD $(OPTIONS) -DF_CPU=$(TEENSY_CORE_SPEED) -Isrc -I$(COREPATH)

# compiler options for C++ only
CXXFLAGS = -std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti

# compiler options for C only
CFLAGS =

# compiler options specific to teensy version
ifeq ($(TEENSY), 30)
    CPPFLAGS += -D__MK20DX128__
    LDSCRIPT = $(COREPATH)/mk20dx128.ld
else
    ifeq ($(TEENSY), 31)
        CPPFLAGS += -D__MK20DX256__
        LDSCRIPT = $(COREPATH)/mk20dx256.ld
    else
        $(error Invalid setting for TEENSY)
    endif
endif

# set arduino define if given
ifdef ARDUINO
	CPPFLAGS += -DARDUINO=$(ARDUINO)
endif

# linker options
LDFLAGS = -Os -Wl,--gc-sections -mcpu=cortex-m4 -mthumb -T$(LDSCRIPT)

#CROSS=/Applications/Arduino.app/Contents/Resources/Java/hardware/tools/arm-none-eabi/bin/arm-none-eabi-

# Uncomment to have gcc filter out unused functions to reduce text size
#CPPFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,--gc-sections

# additional libraries to link
LIBS = -lm -larm_cortexM4l_math

# names for the compiler programs
CC = $(abspath $(COMPILERPATH))/arm-none-eabi-gcc
CXX = $(abspath $(COMPILERPATH))/arm-none-eabi-g++
OBJCOPY = $(abspath $(COMPILERPATH))/arm-none-eabi-objcopy
SIZE = $(abspath $(COMPILERPATH))/arm-none-eabi-size

# automatically create lists of the sources and objects
LOCALLIBLIST = $(wildcard $(LIBRARYPATH)/*)
ARDUINOLIBLIST = $(foreach lib,$(ARDUINOLIBRARIES),$(ARDUINOLIBRARYPATH)/$(lib))
LIBLIST = $(ARDUINOLIBLIST) $(LOCALLIBLIST)
#LC_FILES := $(wildcard $(LIBRARYPATH)/*/*.c $(LIBRARYPATH)/*/utility/*.c)
#LCPP_FILES := $(wildcard $(LIBRARYPATH)/*/*.cpp $(LIBRARYPATH)/*/utility/*.cpp)
LC_FILES := $(foreach lib,$(LIBLIST),$(wildcard $(lib)/*.c $(lib)/utility/*.c))
LCPP_FILES := $(foreach lib,$(LIBLIST),$(wildcard $(lib)/*.cpp $(lib)/utility/*.cpp))
TC_FILES := $(wildcard $(COREPATH)/*.c)
TCPP_FILES := $(wildcard $(COREPATH)/*.cpp)
C_FILES := $(wildcard src/*.c)
CPP_FILES := $(wildcard src/*.cpp)
INO_FILES := $(wildcard src/*.ino)

# include paths for libraries
#L_INC := $(foreach lib,$(filter %/, $(wildcard $(LIBRARYPATH)/*/ $(LIBRARYPATH)/*/utility/)), -I$(lib))
L_INC := $(foreach lib,$(LIBLIST), -I$(lib)) $(foreach lib,$(LIBLIST), -I$(lib)/utility)

SOURCES := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o) $(INO_FILES:.ino=.o) $(TC_FILES:.c=.o) $(TCPP_FILES:.cpp=.o) $(LC_FILES:.c=.o) $(LCPP_FILES:.cpp=.o)
OBJS := $(foreach src,$(SOURCES), $(BUILDDIR)/$(src))

all: hex

build: $(TARGET).elf

hex: $(TARGET).hex

post_compile: $(TARGET).hex
	@$(abspath $(TOOLSPATH))/teensy_post_compile -file="$(basename $<)" -path=$(CURDIR) -tools="$(abspath $(TOOLSPATH))"

reboot:
	@-$(abspath $(TOOLSPATH))/teensy_reboot

upload: post_compile reboot

$(BUILDDIR)/%.o: %.c
	@echo "[CC]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(L_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.cpp
	@echo "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.ino
	@echo "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -x c++ -include Arduino.h -c "$<"

$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@echo "[LD]\t$@"
	@$(CC) $(LDFLAGS) -o "$@" $(OBJS) $(LIBS) -Wl,-Map,$@.map

%.hex: %.elf
	@echo "[HEX]\t$@"
	@$(SIZE) "$<"
	@$(OBJCOPY) -O ihex -R .eeprom "$<" "$@"

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	@echo Cleaning...
	@rm -rf "$(BUILDDIR)"
	@rm -f "$(TARGET).elf" "$(TARGET).hex"
