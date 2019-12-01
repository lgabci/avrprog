noinst_PROGRAMS = avrprog.hex avrprog.elf flashed

avrprog_elf_SOURCES = main.c lcd.c message.c misc.c prog.c serial.c spi.c

avrprog_hex_SOURCES =
avrprog_hex_DEPENDENCIES = avrprog.elf
avrprog_hex_LINK = $(OBJCOPY) $(OBJCOPYFLAGS) $< $@

flashed_SOURCES =
flashed_DEPENDENCIES = avrprog.hex
flashed_LINK = set -e; $(AD) $(AM_ADVFLAGS) || $(AD) $(AM_ADWFLAGS); touch $@
