##
## Process with automake to produce Makefile.in
##

noinst_PROGRAMS = test-client

INCLUDES = -I$(top_srcdir)/src/ @SIGC_CFLAGS@

test_client_SOURCES = test-client.cpp
test_client_LDADD = -L$(top_srcdir)/src/ -licq2000
test_client_DEPENDENCIES = $(top_srcdir)/src/.libs/libicq2000.a
