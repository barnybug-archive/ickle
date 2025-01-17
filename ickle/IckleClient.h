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

#include <sigc++/signal.h>

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
#include "MessageQueue.h"
#include "EventSystem.h"

#ifdef CONTROL_SOCKET
# include "ControlHandler.h"
#endif

class IckleClient : public sigslot::has_slots<>,
		    public SigC::Object
{
#ifdef CONTROL_SOCKET
  friend class ControlHandler;
#endif
 private:
  MessageQueue m_message_queue;
  EventSystem m_event_system;

  IckleGUI gui;
  ICQ2000::Status status;
  unsigned int m_retries;
  std::string auto_response;

  bool m_loading;

#ifdef CONTROL_SOCKET
  ControlHandler ctrl;
#endif

  IdleTimer m_idletimer;

  // setting and history files for each contact, indexed through UINs
  std::map<unsigned int, std::string> m_settingsmap;
  std::map<unsigned int, History *> m_histmap;

  std::map<int, SigC::Connection> m_sockets;
  SigC::Connection poll_server_cnt;

  void processCommandLine(int argc, char* argv[]);
  void usageInstructions(const char* progname);

  void loadSettings();
  void saveSettings();
  void initLibrary();

  void loadContactList();

  void loadSelfContact();
  void saveSelfContact();

  void loadContact(const std::string& s, bool self);
  void saveContact(ICQ2000::ContactRef c, const std::string& s, bool self);

  void event_system(const std::string& s, ICQ2000::ContactRef c, time_t t);
  bool idle_connect_cb(ICQ2000::Status s, bool inv);
  MessageEvent* convert_libicq2000_event(ICQ2000::MessageEvent *ev);

  void SignalLog(ICQ2000::LogEvent::LogType type, const std::string& msg);

  void logger_file_cb(const std::string& msg);

  std::string get_unique_historyname() throw (std::runtime_error);
  std::string get_unique_contact_user_filename(ICQ2000::ContactRef& c);
  std::string get_contact_history_filename(ICQ2000::ContactRef& c);
  void update_group_settings();

  bool mkdir_BASE_DIR();

 public:
  IckleClient(int argc, char* argv[]);
  ~IckleClient();

  void init();
  bool check_pid_file();
  void quit();
  void apply_and_restart(); //reload settings and restart the client

  // -- Callbacks for libICQ2000 --
  void connected_cb(ICQ2000::ConnectedEvent *c);
  void disconnected_cb(ICQ2000::DisconnectedEvent *c);
  void logger_cb(ICQ2000::LogEvent *c);
  void contactlist_cb(ICQ2000::ContactListEvent *ev);
  void message_cb(ICQ2000::MessageEvent* ev);
  void messageack_cb(ICQ2000::MessageEvent* ev);
  void status_change_cb(ICQ2000::StatusChangeEvent* ev);
  void socket_cb(ICQ2000::SocketEvent* ev);
  void want_auto_resp_cb(ICQ2000::ICQMessageEvent *ev);
  void ft_incoming_cb(ICQ2000::FileTransferEvent* ev);
  void ft_update_cb(ICQ2000::FileTransferEvent* ev);

  // -- Callbacks for message queue --
  void queue_added_cb(MessageEvent *ev);

  // -- Callbacks for GUI --
  void send_event_cb(ICQ2000::MessageEvent *ev);
  void fetch_cb(ICQ2000::ContactRef c);
  void exit_cb();
  gint close_cb(GdkEventAny*);
  void save_settings_cb();

  // -- Callbacks for translator --
  void contact_encoding_changed_cb(unsigned int uin);

  // -- Callback for a socket ready --
  bool socket_select_cb(Glib::IOCondition cond, int source);

  // -- Callback for timeout
  bool poll_server_cb();
};

#endif
