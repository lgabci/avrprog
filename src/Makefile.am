noinst_PROGRAMS = avrprog.hex avrprog.elf prog
check_PROGRAMS = test test0 test1

avrprog_elf_SOURCES = main.c hd44780.c message.c misc.c prog.c serial.c spi.c

avrprog_hex_SOURCES =
avrprog_hex_DEPENDENCIES = avrprog.elf
avrprog_hex_LINK = $(OBJCOPY) $(OBJCOPYFLAGS) $< $@

.PHONY: prog$(EXEEXT)
prog_SOURCES =
prog_DEPENDENCIES = avrprog.hex
prog_LINK = $(AD) $(AM_ADVFLAGS) || $(AD) $(AM_ADWFLAGS)

.PHONY: test$(EXEEXT)
test_SOURCES =
test_DEPENDENCIES = avrprog.hex
test_LINK = avrdude -p m8 -c stk500v2 -P /dev/ttyUSB0 -U signature:r:/dev/null:h -vv

.PHONY: test1$(EXEEXT)
test0_SOURCES =
test0_DEPENDENCIES = avrprog.hex
test0_LINK = screen /dev/ttyUSB0 115200

.PHONY: test2$(EXEEXT)
test1_SOURCES =
test1_DEPENDENCIES = avrprog.hex
test1_LINK = avrdude -p m8 -B 9600 -c ft232r -P usb:ft0 -U lfuse:r:/tmp/lfuse.txt:h -vv; cat /tmp/lfuse.txt; rm /tmp/lfuse.txt

TESTS = test
