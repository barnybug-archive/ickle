##
## Process with automake to produce Makefile.in
##

EXTRA_PROGRAMS = ickle ickle_applet
bin_PROGRAMS = @ICKLE_PROGRAM@

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" -I$(top_srcdir)/share/icons/ickle/  \
              @SIGC_CFLAGS@ @GTKMM_CFLAGS@ @GNOME_INCLUDEDIR@ @LIBICQ2000_CFLAGS@ \
              -I$(top_srcdir)/ctrlsocket
LDFLAGS = @SIGC_LIBS@ @GTKMM_LIBS@ @LIBICQ2000_LIBFLAGS@ @XSS_LIBS@ \
          -L$(top_builddir)/ctrlsocket @LIBICKLE_CTRLSOCKET_FLAG@
LDADD = @LIBICQ2000_LIBADD@

ickle_SOURCES = main.cpp IckleClient.cpp IckleGUI.cpp			\
  Settings.cpp ContactListView.cpp MessageBox.cpp AddUserDialog.cpp	\
  Icons.cpp UserInfoDialog.cpp AddMobileUserDialog.cpp			\
  SettingsDialog.cpp PromptDialog.cpp History.cpp MobileNoEntry.cpp	\
  Dir.cpp AwayMessageDialog.cpp EventSubstituter.cpp			\
  SetAutoResponseDialog.cpp SearchDialog.cpp AboutDialog.cpp		\
  IdleTimer.cpp gtkspell.cpp StatusMenu.cpp WizardDialog.cpp		\
  ControlHandler.cpp
ickle_DEPENDENCIES = @LIBICKLE_CTRLSOCKET_DEP@

noinst_HEADERS = main.h IckleClient.h IckleGUI.h Settings.h	\
  ContactListView.h MessageBox.h AddUserDialog.h		\
  Icons.h UserInfoDialog.h AddMobileUserDialog.h		\
  SettingsDialog.h PromptDialog.h History.h			\
  MobileNoEntry.h Dir.h IckleApplet.h				\
  AwayMessageDialog.h sstream_fix.h				\
  SetAutoResponseDialog.h EventSubstituter.h SearchDialog.h     \
  AboutDialog.h IdleTimer.h gtkspell.h StatusMenu.h 		\
  WizardDialog.h ControlHandler.h

ickle_applet_SOURCES = $(ickle_SOURCES) IckleApplet.cpp
ickle_applet_LDFLAGS = @GNOME_APPLETS_LIBS@
ickle_applet_DEPENDENCIES = $(ickle_DEPENDENCIES)

corbadir = $(sysconfdir)/CORBA/servers
desktopdir = $(datadir)/applets/Network
EXTRA_DIST = ickle_applet.gnorba ickle_applet.desktop ickle.1 ickle_applet.1
corba_DATA= @CORBA_FILES@
desktop_DATA= @DESKTOP_FILES@
man_MANS = @ICKLE_MANPAGES@
