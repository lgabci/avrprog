# AVR programmer test program meson file

avrprog_elf = executable('testavrprog.elf', 'main.c')

testhex = custom_target('testavrprog.hex',
  build_by_default: true,
  input : avrprog_elf,
  output : 'testavrprog.hex',
  command : [objcopy, '-j', '.text', '-j', '.data', '-O', 'ihex', '@INPUT@',
    '@OUTPUT@'
  ]
)

run_target('t',
  command : [test, testhex]
)
