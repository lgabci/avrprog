# AVR programmer main meson file

project('avrprog', 'c')
assert(meson.is_cross_build(), 'This project must cross compiled!')

objcopy = find_program('avr-objcopy')
flash = find_program('flash.sh')
test = find_program('test.sh')

## =================================
s = find_program('s.sh')
## =================================

subdir('src')
subdir('testsrc')
