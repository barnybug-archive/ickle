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

#include <gtk--/main.h>
#include <sigc++/signal_system.h>

#include <hash_map>
#include <list>
#include <sstream>

#include <stdlib.h>
#include <getopt.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "main.h"
#include "Client.h"
#include "events.h"
#include "IckleGUI.h"
#include "Settings.h"
#include "SettingsDialog.h"
#include "constants.h"
#include "Icons.h"

using SigC::slot;

using namespace ICQ2000;

class IckleClient : public SigC::Object {
 private:
  IckleGUI gui;
  Settings settings;
  Status status;

  hash_map<unsigned int, string> m_fmap;

  Connection socket_read_cb_cnt, ping_server_cnt;
  int socket_read_cb_fd;

  void processCommandLine(int argc, char* argv[]);
  void loadSettings();
  void saveSettings();

  void loadContactList();

  void event_system(const string& s);

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

  // -- Callbacks for GUI --
  void status_change_cb(Status st);
  void user_popup_cb(unsigned int uin);
  void user_info_cb(unsigned int uin);
  void send_event_cb(MessageEvent *ev);
  void add_user_cb(unsigned int uin);
  void add_mobile_user_cb(string,string);
  void settings_cb();

  // -- Callback for socket ready for read --
  void socket_read_cb(int source, GdkInputCondition cond);

  // -- Callback for timeout
  int ping_server_cb();

};

#endif
