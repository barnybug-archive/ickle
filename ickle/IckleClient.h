/*
 * IckleClient (the controller)
 * The business end of Ickle, handles all the events
 * the library and tells the view and model parts what to do.
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef ICKLECLIENT_H
#define ICKLECLIENT_H

#include <sigc++/signal_system.h>

#include <map>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <map>

#include <libicq2000/constants.h>
#include <libicq2000/events.h>

#include "main.h"
#include "IckleGUI.h"
#include "Settings.h"
#include "History.h"
#include "IdleTimer.h"

using std::map;

#ifdef GNOME_ICKLE
# include "IckleApplet.h"
#endif

using SigC::slot;

using Gtk::Connection;

using namespace ICQ2000;

class IckleClient : public SigC::Object {
 private:
  IckleGUI gui;
  Status status;
  unsigned int m_retries;
  string auto_response;

#ifdef GNOME_ICKLE
  IckleApplet applet;
#endif

  IdleTimer m_idletimer;
  
  // setting and history files for each contact, indexed through UINs
  std::map<unsigned int, string> m_settingsmap; 
  std::map<unsigned int, History *> m_histmap;

  std::map<int, Connection> m_sockets;
  Connection poll_server_cnt;

  void processCommandLine(int argc, char* argv[]);
  void usageInstructions(const char* progname);

  void loadSettings();
  void saveSettings();

  void loadContactList();

  void event_system(const string& s, MessageEvent *e);
  gint idle_connect_cb(Status s);

  void SignalLog(LogEvent::LogType type, const string& msg);

  void logger_file_cb(const string& msg);

  string get_unique_historyname() throw (runtime_error);

 public:
  IckleClient(int argc, char* argv[]);
  ~IckleClient();
  
  void quit();

  // -- Callbacks for libICQ2000 --
  void connected_cb(ConnectedEvent *c);
  void disconnected_cb(DisconnectedEvent *c);
  void logger_cb(LogEvent *c);
  void contactlist_cb(ContactListEvent *ev);
  bool message_cb(MessageEvent* ev);
  void messageack_cb(MessageEvent* ev);
  void socket_cb(SocketEvent* ev);
  void want_auto_resp_cb(AwayMessageEvent *ev);

  // -- Callbacks for GUI --
  void user_popup_cb(unsigned int uin);
  void userinfo_cb(unsigned int uin);
  void send_event_cb(MessageEvent *ev);
  void add_user_cb(unsigned int uin);
  void add_mobile_user_cb(string,string);
  void fetch_cb(Contact *c);
  void exit_cb();
  gint close_cb(GdkEventAny*);
  void settings_changed_cb();

  // -- Callback for a socket ready --
  void socket_select_cb(int source, GdkInputCondition cond);

  // -- Callback for timeout
  int poll_server_cb();
};

#endif
