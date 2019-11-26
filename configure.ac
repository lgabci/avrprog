AC_PREREQ([2.69])
AC_INIT([avrprog], [0.0.1], [])

AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC([gcc])
AC_LANG([C])

AC_CHECK_TOOL([OBJCOPY], [objcopy], [:])
if test ${OBJCOPY} == :; then
  AC_MSG_ERROR([Program not found: objcopy])
fi

AC_CHECK_PROG([AD], [avrdude], [avrdude], [:])
if test ${AD} == :; then
  AC_MSG_ERROR([Program not found: avrdude])
fi
AC_SUBST([AM_ADVFLAGS], ["-p m8 -c ft232r -P usb:ft0 -qq -U flash:v:$<:i"])
AC_SUBST([AM_ADWFLAGS], ["-p m8 -c ft232r -P usb:ft0 -U flash:w:$<:i"])

AC_SUBST([AM_CFLAGS], ["-mmcu=atmega8a -O2 -pedantic -Wall -Werror"])
AC_SUBST([OBJCOPYFLAGS], ["-j .text -j .data -O ihex"])

AC_CONFIG_FILES([Makefile
		 src/Makefile])

AC_OUTPUT
