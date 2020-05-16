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

# flash test uC
testflash = custom_target('testflash',
  output: 'testflash',
  input: testavrprog_hex,
  command: [flash_sh, '@INPUT@', 'v', 'stk500v2', '1MHz'],
  build_by_default: false
)

