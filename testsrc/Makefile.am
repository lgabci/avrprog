noinst_PROGRAMS = avrprog.hex avrprog.elf

avrprog_elf_SOURCES = main.c

avrprog_hex_SOURCES =
avrprog_hex_DEPENDENCIES = avrprog.elf
avrprog_hex_LINK = $(OBJCOPY) $(OBJCOPYFLAGS) $< $@
