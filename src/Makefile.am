##
## Process with automake to produce Makefile.in
##

INCLUDES = @SIGC_CFLAGS@
LDFLAGS = @SIGC_LIBS@

lib_LTLIBRARIES = libicq2000.la

libicq2000_la_SOURCES = Client.cpp TLV.cpp buffer.cpp events.cpp \
                        socket.cpp SNAC.cpp Xml.cpp Contact.cpp \
			ContactList.cpp exceptions.cpp
pkginclude_HEADERS = Client.h buffer.h socket.h TLV.h constants.h \
                     SNAC.h events.h Xml.h Contact.h \
                     ContactList.h exceptions.h custom_marshal.h
libicq2000_la_LDFLAGS = -version-info 1:0:0

