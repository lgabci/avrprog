#!/bin/bash
set -e

cd "$MESON_BUILD_ROOT"

hex="$1"
trg=flashed

if [ -z "$hex" ]; then
  echo "Missing hex file name" >&2
  exit 1
fi

if [ "$hex" -nt "$trg" ]; then
  VERIFY="-p m8 -c ft232r -P usb:ft0 -qq -U flash:v:$hex:i"
  WRITE="-p m8 -c ft232r -P usb:ft0 -U flash:w:$hex:i"

  avrdude $VERIFY || avrdude $WRITE

  touch "$trg"
fi
