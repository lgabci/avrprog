# AVR programmer makefile
#

CC=avr-gcc
CFLAGS=-mmcu=atmega8a -O2 -pedantic -Wall -Werror

all: prog

avrprog.elf: avrprog.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: prog
prog: avrprog.elf
	avrdude -p m8 -c ft232r -P ft0 -U flash:v:$<:e || avrdude -p m8 -c ft232r -P ft0 -U flash:w:$<:e


.PHONY: test
test: avrprog.elf
	avrdude -p m8 -c stk500v2 -P /dev/ttyUSB0 -U signature:r:/dev/null:h
