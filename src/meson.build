# AVR programmer meson file

avrprog_elf = executable('avrprog.elf', 'main.c', 'message.c', 'misc.c',
  'prog.c', 'serial.c', 'spi.c', 'lcd.c')

hex = custom_target('avrprog.hex',
  build_by_default: true,
  input : avrprog_elf,
  output : 'avrprog.hex',
  command : [objcopy, '-j', '.text', '-j', '.data', '-O', 'ihex', '@INPUT@',
    '@OUTPUT@'
  ]
)

run_target('flash',
  depends: hex,
  command : [flash, hex.full_path()]
)
