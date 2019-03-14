ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = m4/gnulib-cache.m4
SUBDIRS = lib

bin_PROGRAMS = rether

rether_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
rether_CFLAGS = $(POPT_CFLAGS)
rether_LDADD = lib/libgnu.a $(POPT_LIBS)
rether_SOURCES = args.c send.c receive.c main.c
