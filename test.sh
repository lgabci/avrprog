#!/bin/bash
set -e

cd "$MESON_BUILD_ROOT"
prog="$1"
if [ -z "$prog" ]; then
  echo "Missing prog." >2&
  exit 1
fi

case "$2" in
  ft)
    avrdude -p m8 -c ft232r -P usb:ft0 -E reset -v
    ;;
  *)
    USBDEV=/dev/ttyUSB0
    if [ ! -e $USBDEV ] && lsusb -d 0403:6001 >/dev/null; then
      sudo rmmod ftdi_sio && sudo modprobe ftdi_sio
    fi
    [ "$1" == w ] && com=w || com=v
    avrdude -p m8 -c stk500v2 -P $USBDEV -v -U flash:$com:$prog:i
    ;;
esac
#screen /dev/ttyUSB0 115200
#avrdude -p m8 -B 9600 -c ft232r -P usb:ft0 -U lfuse:r:/tmp/lfuse.txt:h -vv; cat /tmp/lfuse.txt; rm /tmp/lfuse.txt
