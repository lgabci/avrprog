#!/bin/bash
set -e

if [ "$1" == ft ]; then
  avrdude -p m8 -c ft232r -P usb:ft0 -E reset -v
else
  USBDEV=/dev/ttyUSB0
  if [ ! -e  $USBDEV ] && lsusb -d 0403:6001 >/dev/null; then
    sudo rmmod ftdi_sio && sudo modprobe ftdi_sio
  fi
  prog=$(pwd)/testsrc/avrprog.hex
  avrdude -p m8 -c stk500v2 -P $USBDEV -v -U flash:w:$prog:i
fi
#screen /dev/ttyUSB0 115200
#avrdude -p m8 -B 9600 -c ft232r -P usb:ft0 -U lfuse:r:/tmp/lfuse.txt:h -vv; cat /tmp/lfuse.txt; rm /tmp/lfuse.txt
