/*
 * MessageBox
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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <string>

#include <gtk--/window.h>
#include <gtk--/table.h>
#include <gtk--/text.h>
#include <gtk--/paned.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/buttonbox.h>
#include <gtk--/notebook.h>
#include <gtk--/entry.h>
#include <gtk--/statusbar.h>
#include <gtk--/togglebutton.h>

#include <time.h>

#include <sigc++/signal_system.h>

#include "Contact.h"
#include "events.h"

#include "Icons.h"

using namespace ICQ2000;

using std::string;
using SigC::Signal1;

class MessageBox : public Gtk::Window {
 private:
  Contact *m_contact;

  Gtk::VBox m_vbox_top;
  Gtk::Button m_send_button, m_close_button;

  Gtk::Table m_history_table;
  Gtk::Text m_history_text;

  Gtk::Notebook m_tab;

  // normal message tab
  Gtk::Text m_message_text;

  // url tab
  Gtk::Text m_url_text;
  Gtk::Entry m_url_entry;

  // sms tab
  Gtk::Text m_sms_text;
  Gtk::Entry m_sms_count;
  Gtk::Label m_sms_count_label;
  bool m_sms_count_over;
  bool m_sms_enabled, m_online;
  bool m_display_times;
  
  Gtk::VPaned m_pane;
  Gtk::ToggleButton *m_userinfo_toggle;

  Gtk::Statusbar m_status;
  guint m_status_context;

  MessageEvent::MessageType m_message_type;

  void send_button_update();
  void set_contact_title();
  string format_time(time_t t);
  void display_message(MessageEvent *ev, bool sent, const string& nick);
  void set_status( const string& text );

 public:
  MessageBox(Contact *c);
  ~MessageBox();

  bool message_cb(MessageEvent *ev);
  void messageack_cb(MessageEvent *ev);
  void contactlist_cb(ContactListEvent *ev);

  void enable_sms();
  void disable_sms();

  void online();
  void offline();

  void userinfo_dialog_cb(bool b);

  void setDisplayTimes(bool d);
  
  void raise() const;

  // signals
  SigC::Signal1<void,MessageEvent *> send_event;
  SigC::Signal1<void,bool> userinfo_dialog;

  void send_clicked_cb();
  void switch_page_cb(Gtk::Notebook_Helpers::Page* p, guint n);
  void userinfo_toggle_cb();
  void sms_count_update_cb();
  gint key_press_cb(GdkEventKey*);
  virtual gint delete_event_impl(GdkEventAny *ev);
};

#endif
