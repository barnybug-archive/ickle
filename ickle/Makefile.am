##
## Process with automake to produce Makefile.in
##

bin_PROGRAMS = ickle

# set gettext LOCALEDIR
datadir = @datadir@
localedir = $(datadir)/locale
AM_CPPFLAGS = -I$(top_srcdir)/share/icons/ickle/ -DPKGDATADIR=\"$(pkgdatadir)\"		\
              @GTKMM_CFLAGS@ @LIBICQ2000_CFLAGS@	\
              -I$(top_srcdir)/ctrlsocket -DLOCALEDIR=\"$(localedir)\"

LDFLAGS = @GTKMM_LIBS@ @LIBICQ2000_LIBS@ @XSS_LIBS@ \
          -L$(top_builddir)/ctrlsocket @LIBICKLE_CTRLSOCKET_FLAG@ \
          @LIBINTL@

ickle_SOURCES = main.cpp IckleClient.cpp IckleGUI.cpp			\
  Settings.cpp ContactListView.cpp MessageBox.cpp			\
  Icons.cpp UserInfoDialog.cpp PromptDialog.cpp				\
  SettingsDialog.cpp History.cpp MobileNoEntry.cpp			\
  Dir.cpp AwayMessageDialog.cpp EventSubstituter.cpp			\
  SetAutoResponseDialog.cpp SearchDialog.cpp AboutDialog.cpp		\
  IdleTimer.cpp StatusMenu.cpp WizardDialog.cpp				\
  ControlHandler.cpp MessageEvent.cpp MessageQueue.cpp EventSystem.cpp	\
  SendAuthReqDialog.cpp AuthRespDialog.cpp ResendDialog.cpp		\
  AddGroupDialog.cpp RenameGroupDialog.cpp RemoveGroupDialog.cpp	\
  AddContactDialog.cpp RemoveContactDialog.cpp LogWindow.cpp utils.cpp	\
  UserInfoHelpers.cpp Translator.cpp SetEncodingDialog.cpp

ickle_DEPENDENCIES = @LIBICKLE_CTRLSOCKET_DEP@

noinst_HEADERS = main.h IckleClient.h IckleGUI.h Settings.h		\
  ContactListView.h MessageBox.h PromptDialog.h				\
  Icons.h UserInfoDialog.h						\
  SettingsDialog.h History.h						\
  MobileNoEntry.h Dir.h							\
  AwayMessageDialog.h sstream_fix.h					\
  SetAutoResponseDialog.h EventSubstituter.h SearchDialog.h		\
  AboutDialog.h IdleTimer.h StatusMenu.h				\
  WizardDialog.h ControlHandler.h MessageEvent.h MessageQueue.h		\
  EventSystem.h SendAuthReqDialog.h AuthRespDialog.h ResendDialog.h	\
  AddGroupDialog.h RenameGroupDialog.h RemoveGroupDialog.h		\
  AddContactDialog.cpp RemoveContactDialog.cpp LogWindow.h utils.h	\
  UserInfoHelpers.h Translator.h SetEncodingDialog.h

man_MANS = ickle.1
EXTRA_DIST = ickle.1

