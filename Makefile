# AVR programmer makefile
#

CC := avr-gcc
CFLAGS := -mmcu=atmega8a -O2 -pedantic -Wall -Werror

OBJCOPY := avr-objcopy
OBJCOPYFLAGS := -j .text -j .data -O ihex

all: prog

avrprog.elf: avrprog.c command.h
	$(CC) $(CFLAGS) -O2 -o $@ $<
	@chmod -x $@

avrprog.hex: avrprog.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

.PHONY: prog
prog: avrprog.hex
	avrdude -p m8 -c ft232r -P ft0 -qq -U flash:v:$<:i || avrdude -p m8 -c ft232r -P ft0 -U flash:w:$<:i
#	avrdude -p m8 -c ft232r -P ft0 -qq -U hfuse:v:0xd9:m || avrdude -p m8 -c ft232r -P ft0 -U hfuse:w:0xd9:m
#	avrdude -p m8 -c ft232r -P ft0 -qq -U lfuse:v:0xff:m || avrdude -p m8 -c ft232r -P ft0 -U lfuse:w:0xff:m

.PHONY: clean
clean:
	rm -f avrprog.elf avrprog.hex

.PHONY: test
test: avrprog.elf
	avrdude -p m8 -c stk500v2 -P /dev/ttyUSB0 -U signature:r:/dev/null:h -vv

.PHONY: test2
test2:
	avrdude -p m8 -B 9600 -c ft232r -P ft0 -U lfuse:r:/tmp/lfuse.txt:h -vv
	cat /tmp/lfuse.txt
