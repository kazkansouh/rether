#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rether], [0.1], [github.com/kazkansouh])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
gl_EARLY

# Checks for libraries.
PKG_CHECK_MODULES([POPT], [popt])
AC_SUBST([POPT_CFLAGS])
AC_SUBST([POPT_LIBS])

# Checks for header files.
gl_INIT

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Setup global CFLAGS
CFLAGS="${CFLAGS} -Wall -Wpedantic -Werror -std=c11"

AC_CONFIG_FILES([Makefile lib/Makefile])
AC_OUTPUT
