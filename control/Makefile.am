##
## Process with automake to produce Makefile.in
##

EXTRA_PROGRAMS = ickle_control
bin_PROGRAMS = @ICKLE_CONTROL@

AM_CPPFLAGS = -I$(top_srcdir)/ctrlsocket @LIBICQ2000_CFLAGS@ @GLIBMM_CFLAGS@ \
	      -DLOCALEDIR=\"$(localedir)\"

LDFLAGS = -L$(top_builddir)/ctrlsocket -lickle_ctrlsocket @GLIBMM_LIBS@ @LIBINTL@

ickle_control_SOURCES = main.cpp IckleControl.cpp CommandLineParser.cpp
noinst_HEADERS = IckleControl.h CommandLineParser.h
ickle_control_DEPENDENCIES = @LIBICKLE_CTRLSOCKET_DEP@

EXTRA_DIST = ickle_control.1
man_MANS = @ICKLE_CONTROL_MANPAGES@
