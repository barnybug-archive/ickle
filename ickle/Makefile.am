##
## Process with automake to produce Makefile.in
##

EXTRA_PROGRAMS = ickle ickle_applet
bin_PROGRAMS = @ICKLE_PROGRAM@

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" -I$(top_srcdir)/src/ -I$(top_srcdir)/share/icons/ickle/  \
              @SIGC_CFLAGS@ @GTKMM_CFLAGS@ @GNOME_INCLUDEDIR@
LDFLAGS = @SIGC_LIBS@ @GTKMM_LIBS@

ickle_SOURCES = main.cpp IckleClient.cpp IckleGUI.cpp \
                Settings.cpp ContactListView.cpp \
		MessageBox.cpp AddUserDialog.cpp \
                Icons.cpp UserInfoDialog.cpp \
		AddMobileUserDialog.cpp SettingsDialog.cpp \
		PromptDialog.cpp History.cpp MobileNoEntry.cpp \
                Dir.cpp AwayMessageDialog.cpp
noinst_HEADERS = main.h IckleClient.h IckleGUI.h Settings.h \
                 ContactListView.h MessageBox.h AddUserDialog.h \
                 Icons.h UserInfoDialog.h AddMobileUserDialog.h \
		 SettingsDialog.h PromptDialog.h History.h \
		 MobileNoEntry.h Dir.h IckleApplet.h AwayMessageDialog.h
ickle_LDADD = $(top_srcdir)/src/libicq2000.la

ickle_applet_SOURCES = $(ickle_SOURCES) IckleApplet.cpp
ickle_applet_LDADD = $(ickle_LDADD)
ickle_applet_LDFLAGS = @GNOME_APPLETS_LIBS@

corbadir = $(sysconfdir)/CORBA/servers
desktopdir = $(datadir)/applets/Network
EXTRA_DIST = ickle_applet.gnorba ickle_applet.desktop
corba_DATA= @CORBA_FILES@
desktop_DATA= @DESKTOP_FILES@
