ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = m4/gnulib-cache.m4
AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
SUBDIRS = lib
LDADD = lib/libgnu.a

bin_PROGRAMS = rether

rether_SOURCES = main.c
