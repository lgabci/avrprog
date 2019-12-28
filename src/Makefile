# AVR programmer makefile

PROG := avrprog.elf
PROGH := $(PROG:.elf=.hex)

all: $(PROG)

CC := avr-gcc
CFLAGS := -mmcu=atmega8a -O2 -pedantic -Wall -Werror -c

LD := $(CC)
LDFLAGS := -mmcu=atmega8a

OBJCOPY := avr-objcopy
OBJCOPYFLAGS := -j .text -j .data -O ihex

AD := avrdude
ADVERFLAGS = -p m8 -c ft232r -P usb:ft0 -qq -U flash:v:$<:i
ADWRTFLAGS = -p m8 -c ft232r -P usb:ft0 -U flash:w:$<:i

# dont allow to compile in source directory
MKFILEDIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
ifeq ($(MKFILEDIR), $(CURDIR)/)
  $(error "Only out of tree buildings allowed!")
endif

# don't use any suffix rule
.SUFFIXES:

VPATH := $(MKFILEDIR)

SOURCES := main.c message.c misc.c prog.c serial.c spi.c led.c
OBJS := $(SOURCES:.c=.o)

%.hex:
	$(OBJCOPY) $(OBJCOPYFLAGS) $^ $@

%.elf:
	$(LD) $(LDFLAGS) -o $@ $^
	chmod -x $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

$(PROG): $(OBJS)
$(PROGH): $(PROG)

# test -----------------------------------------
USBDEV=/dev/ttyUSB0
TESTADVERFLAGS = -p m8 -B 1MHz -c stk500v2 -P $(USBDEV) -v -U flash:v:$<:i
TESTADWRTFLAGS = -p m8 -B 1MHz -c stk500v2 -P $(USBDEV) -v -U flash:w:$<:i

TESTPROG := testavrprog.elf
TESTPROGH := $(TESTPROG:.elf=.hex)

TESTSOURCES := testmain.c
TESTOBJS := $(TESTSOURCES:.c=.o)

$(TESTPROG): $(TESTOBJS)
$(TESTPROGH): $(TESTPROG)

flash: $(PROGH)
	$(AD) $(ADVERFLAGS) || $(AD) $(ADWRTFLAGS)
	touch $@

testflash: $(TESTPROGH) usbdev
	$(AD) $(TESTADVERFLAGS)

testflashw: $(TESTPROGH) usbdev
	$(AD) $(TESTADWRTFLAGS)

.PHONY: usbdev
usbdev:
	if [ ! -e $(USBDEV) ] && lsusb -d 0403:6001 >/dev/null; then \
	  sudo rmmod ftdi_sio && sudo modprobe ftdi_sio; \
	fi
# test -----------------------------------------

.PHONY: clean
clean:
	rm -rf $(OBJS) $(TESTOBJS) $(PROG) $(PROGH) $(TESTPROG) $(TESTPROGH)
