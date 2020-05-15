# avrprog top meson file

project('avrprog', 'c',
        meson_version: '>=0.49.2',
        version: '0.0.1'
       )

# enable only cross compiling
assert(meson.is_cross_build(), 'Only cross build enabled!')

# find programs
objcopy = find_program('objcopy')
flash_sh = find_program('flash.sh')

# compile ELF
avrprog_elf = executable('avrprog.elf',
  'main.c', 'message.c', 'misc.c', 'prog.c', 'serial.c', 'spi.c', 'led.c'
)

# test program
subdir('test')

# create HEX file
avrprog_hex = custom_target('avrprog.bin',
  output: 'avrprog.hex',
  input: avrprog_elf,
  command: [objcopy, '-j', '.text', '-j', '.data', '-O', 'ihex',
  '@INPUT@', '@OUTPUT@']
)

# flash uC
flash = custom_target('flash',
  output: 'flash',
  input: avrprog_hex,
  command: [flash_sh, '@INPUT@'],
  build_by_default: true
)
