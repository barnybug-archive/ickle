##
## Process with automake to produce Makefile.in
##

bin_PROGRAMS = ickle

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\"
LDFLAGS = @SIGC_LIBS@ @GTKMM_LIBS@
INCLUDES = -I$(top_srcdir)/src/ -I$(top_srcdir)/icons/  \
           @SIGC_CFLAGS@ @GTKMM_CFLAGS@


ickle_SOURCES = main.cpp IckleClient.cpp IckleGUI.cpp \
                Settings.cpp ContactListView.cpp \
		MessageBox.cpp AddUserDialog.cpp \
                Icons.cpp UserInfoDialog.cpp \
		AddMobileUserDialog.cpp SettingsDialog.cpp \
		PromptDialog.cpp History.cpp MobileNoEntry.cpp
noinst_HEADERS = main.h IckleClient.h IckleGUI.h Settings.h \
                 ContactListView.h MessageBox.h AddUserDialog.h \
                 Icons.h UserInfoDialog.h AddMobileUserDialog.h \
		 SettingsDialog.h PromptDialog.h History.h \
		 MobileNoEntry.h
ickle_LDADD = -L$(top_srcdir)/src/ -licq2000
ickle_DEPENDENCIES = $(top_srcdir)/src/.libs/libicq2000.a

