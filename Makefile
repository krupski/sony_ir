###############################################################################
#
#  Infrared Remote Control for SONY DSLR A-700 Digital Camera
#  Copyright (c) 2016, 2017 Roger A. Krupski <rakrupski@verizon.net>
#
#  Last update: 24 April 2017
#
#  This program is free software: you can redistribute it and#or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################

## avr part
AVR_PART = attiny85

## avr crystal freq
F_CPU = 16000000UL

## tiny85 fuse settings
LFUSE  = 0xFF
HFUSE  = 0xDF
EFUSE  = 0xFF
UNLOCK = 0xFF
LOCK   = 0xFF

## avr programmer defines
PGM_NAME = avrispmkii
PGM_PORT = usb
PGM_BAUD = 115200

## our program of course!
TARGET = $(notdir $(CURDIR))

## strangely, -O3 makes a smaller file than -Os...
OPT = O3

## uncomment these if you need floating point support
## FLOAT_SUPPORT += -Wl,-u,vfprintf,-lprintf_flt
## FLOAT_SUPPORT += -Wl,-u,vfscanf,-lscanf_flt

######################################################################
##################### no changes beyond this point ###################
######################################################################

CPPFLAGS += -Wall
CPPFLAGS += -Wextra
CPPFLAGS += -ffunction-sections
CPPFLAGS += -g
CPPFLAGS += -$(OPT)
CPPFLAGS += -mmcu=$(AVR_PART)
CPPFLAGS += -DF_CPU=$(F_CPU)

LINKFLAGS += -mmcu=$(AVR_PART)
LINKFLAGS += $(FLOAT_SUPPORT)
LINKFLAGS += -Wl,--gc-sections
LINKFLAGS += -lm

AVR_ERASE += $(AVRDUDE)
AVR_ERASE += -p $(AVR_PART)
AVR_ERASE += -b $(PGM_BAUD)
AVR_ERASE += -c $(PGM_NAME)
AVR_ERASE += -P $(PGM_PORT)
AVR_ERASE += -e
AVR_ERASE += -qq

AVRDUDE_CMD += $(AVR_ERASE)
AVRDUDE_CMD += -s
AVRDUDE_CMD += -U lock:w:$(UNLOCK):m
AVRDUDE_CMD += -U lfuse:w:$(LFUSE):m
AVRDUDE_CMD += -U hfuse:w:$(HFUSE):m
AVRDUDE_CMD += -U efuse:w:$(EFUSE):m
AVRDUDE_CMD += -U flash:w:$(TARGET).hex:i
AVRDUDE_CMD += -U eeprom:w:$(TARGET).eep:i
AVRDUDE_CMD += -U lock:w:$(LOCK):m

############ for Windoze ############        %%%%%%
## CC = avr-gcc.exe                         %%%% = =
## CXX = avr-g++.exe                        %%C    >
## LD = avr-ld.exe                           _)' _( .' ,
## AR = avr-ar.exe                        __/ |_/\   " *. o
## OBJCOPY = avr-objcopy.exe             /` \_\ \/     %`= '_  .
## OBJDUMP = avr-objdump.exe            /  )   \/|      .^',*. ,
## NM = avr-nm.exe                     /' /-   o/       - " % '_
## AVRSIZE = avr-size.exe             /\_/     <       = , ^ ~ .
## AVRDUDE = avrdude.exe              )_o|----'|          .`  '
## ECHO = @echo                   ___// (_  - (\
## DELETE = del /F /Q            ///-(    \'   \\

CC = avr-gcc
CXX = avr-g++
LD = avr-ld
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
NM = avr-nm
AVRSIZE = avr-size
AVRDUDE = avrdude
ECHO = echo
DELETE = rm -f

# default rule
.DEFAULT_GOAL = build

#_______________________________________________________________________________
# BUILD RULES

all: erase build upload

build: $(TARGET).hex $(TARGET).eep $(TARGET).lst
	@$(ECHO) "Build of $(TARGET) complete."

erase:
	@$(AVR_ERASE)
	@$(ECHO) "$(PART) erased."

upload:
	@$(AVRDUDE_CMD)
	@$(ECHO) "$(TARGET) uploaded to $(PART)."

clean:
	@$(DELETE) $(TARGET).d
	@$(DELETE) $(TARGET).eep
	@$(DELETE) $(TARGET).elf
	@$(DELETE) $(TARGET).hex
	@$(DELETE) $(TARGET).lst
	@$(DELETE) $(TARGET).o
	@$(ECHO) "Build files deleted."

#_______________________________________________________________________________
# BUILD THE TARGET

%.o: %.cpp
	@$(COMPILE.cpp) $(CPPFLAGS) -o $(@) $(<)

%.o: %.c
	@$(COMPILE.c) $(CPPFLAGS) -o $(@) $(<)

%.elf: %.o
	@$(CC) $(LINKFLAGS) -o $(@) $(<)

%.hex: %.elf
	@$(OBJCOPY) \
	-O ihex \
	-R .eeprom \
	--preserve-dates \
	$(<) $(@)
	@$(AVRSIZE) $(@)
	@$(ECHO)

%.eep: %.elf
	@$(OBJCOPY) \
	-O ihex \
	-j .eeprom \
	--preserve-dates \
	--no-change-warnings \
	--change-section-lma=.eeprom=0 \
	--set-section-flags=.eeprom=alloc,load \
	$(<) $(@)
	@$(AVRSIZE) $(@)
	@$(ECHO)

%.lst: %.elf
	@$(OBJDUMP) \
	--demangle \
	--disassemble \
	--source \
	--wide \
	$(<) > $(@)

####### end of Makefile ########
