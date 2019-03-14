ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = m4/gnulib-cache.m4
AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
SUBDIRS = lib
LDADD = lib/libgnu.a

bin_PROGRAMS = rether

rether_CFLAGS = $(POPT_CFLAGS)
rether_LDFLAGS = $(POPT_LIBS)
rether_SOURCES = args.c send.c receive.c main.c
