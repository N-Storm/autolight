# Project: autolight
# Author: NStorm
# License: CC-BY-SA-3.0 https://creativecommons.org/licenses/by-sa/3.0/
# https://github.com/N-Storm/autolight

DEVICE     = attiny10
F_CPU      = 250000
PROGRAMMER = -c usbasp -P /dev/ttyUSB* -b 115200 
OBJECTS    = autolight.o i2csoft.o

######################################################################
######################################################################

# Tune the lines below only if you know what you are doing:

CC = avr-gcc
SIZE = avr-size
CFLAGS = -Os -std=gnu11 -flto -Wall
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE) -v
COMPILE = $(CC) $(CFLAGS) -DF_CPU=$(F_CPU) -mmcu=$(DEVICE)

# symbolic targets:
all:	autolight.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:autolight.hex:i

fuse:	all flash
	$(AVRDUDE) -U fuse:w:0xfe:m

install: flash

clean:
	rm -f autolight.hex autolight.elf autolight.lss $(OBJECTS)

# file targets:
autolight.elf: $(OBJECTS)
	$(COMPILE) -o autolight.elf $(OBJECTS)

autolight.hex: autolight.elf
	rm -f autolight.hex
	avr-objcopy -j .text -j .data -O ihex autolight.elf autolight.hex
	$(SIZE) -C --mcu=$(DEVICE) autolight.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	autolight.elf
	avr-objdump -d -h -S autolight.elf > autolight.lss

cpp:
	$(COMPILE) -E autolight.c

debug: CFLAGS = -DDEBUG -Og -g -std=gnu11 -Wall
debug: clean autolight.hex disasm

oldstyle:
	$(COMPILE) autolight.c i2csoft.c -o autolight.elf
	avr-objdump -h -S autolight.elf > autolight.lss
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "autolight.elf" "autolight.hex"
	avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "autolight.elf" "autolight.eep" || exit 0
	avr-objcopy -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "autolight.elf" "autolight.srec"
	$(SIZE) -C --mcu=attiny10 autolight.elf
