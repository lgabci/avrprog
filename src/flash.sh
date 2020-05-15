#!/bin/sh
set -eu

if [ $# -le 3 ] || [ $# -gt 4 ]; then
  echo "$(basename $0) filename operator target [speed]" >&2
  echo "  filename: file to flash into uC" >&2
  echo "  operator, v = verify, w = write" >&2
  echo "  target: ft/stk500" >&2
  echo "  speed: speed to pass to avrdude" >&2
  exit 1
fi

FILE="$1"
if ! [ -e "$FILE" ]; then
  echo "File not found: $FILE" >&2
  exit 1
fi

case "$2" in
  v|w)
    OP="$2"
    ;;
  *)
    echo "Invalid operator: $2" >&2
    exit 1
    ;;
esac

case "$3" in
  ft)
    PRGID=ft232r
    PORT=usb:ft0
    ;;
  stk500*)
    PRGID=stk500v2
    PORT=/dev/ttyUSB0
    ;;
  *)
    echo "Invalid target: $3" >&2
    exit 1
    ;;
esac

SPD=
if [ -n "${4:-}" ]; then
  SPD="-B $4"
fi

if [ "$OP" = w ]; then
  if avrdude -p m8 $SPD -c "$PRGID" -P "$PORT" -qq -U flash:v:"$FILE":i; then
    exit
  fi
fi
avrdude -p m8 $SPD -c "$PRGID" -P "$PORT" -U flash:"$OP":"$FILE":i

