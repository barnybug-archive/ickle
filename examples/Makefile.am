##
## Process with automake to produce Makefile.in
##

noinst_PROGRAMS = ickle-shell

LDFLAGS = @SIGC_LIBS@
INCLUDES = -I$(top_srcdir)/src/ \
           @SIGC_CFLAGS@

ickle_shell_SOURCES = shell.cpp
ickle_shell_LDADD = -L$(top_srcdir)/src/ -licq2000
ickle_shell_DEPENDENCIES = $(top_srcdir)/src/.libs/libicq2000.a

