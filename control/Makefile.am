##
## Process with automake to produce Makefile.in
##

EXTRA_PROGRAMS = ickle_control
bin_PROGRAMS = @ICKLE_CONTROL@

AM_CPPFLAGS = -I$(top_srcdir)/ctrlsocket @LIBICQ2000_CFLAGS@
LDFLAGS = -L$(top_builddir)/ctrlsocket -lickle_ctrlsocket

ickle_control_SOURCES = main.cpp IckleControl.cpp CommandLineParser.cpp
noinst_HEADERS = IckleControl.h CommandLineParser.h
ickle_control_DEPENDENCIES = @LIBICKLE_CTRLSOCKET_DEP@

