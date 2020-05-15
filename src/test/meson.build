# avrprog test program meson file

testavrprog_elf = executable('testavrprog.elf',
  'testmain.c',
  build_by_default: false
)

testavrprog_hex = custom_target('testavrprog.bin',
  output: 'testavrprog.hex',
  input: testavrprog_elf,
  command: [objcopy, '-j', '.text', '-j', '.data', '-O', 'ihex',
    '@INPUT@', '@OUTPUT@'],
  build_by_default: false
)

USBDEV = '/dev/ttyUSB0'
#TESTADVERFLAGS = -p m8 -B 1MHz -c stk500v2 -P $(USBDEV) -v -U flash:v:$<:i
#TESTADWRTFLAGS = -p m8 -B 1MHz -c stk500v2 -P $(USBDEV) -v -U flash:w:$<:i

