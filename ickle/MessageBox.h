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

#include <time.h>

#include <sigc++/signal_system.h>

#include "Contact.h"
#include "events.h"

#include "Icons.h"

using namespace ICQ2000;

using std::string;
using SigC::Signal1;

using Gtk::VBox;
using Gtk::HButtonBox;
using Gtk::Button;
using Gtk::Table;
using Gtk::Text;
using Gtk::Entry;
using Gtk::Label;
using Gtk::VPaned;
using Gtk::Notebook;

class MessageBox : public Gtk::Window {
 private:
  Contact *m_contact;

  VBox m_vbox_top;
  HButtonBox m_hbox_buttons;
  Button m_send_button, m_close_button;

  Table m_history_table;
  Text m_history_text;

  Notebook m_tab;

  // normal message tab
  Text m_message_text;

  // url tab
  Text m_url_text;
  Entry m_url_entry;

  // sms tab
  Text m_sms_text;
  Entry m_sms_count;
  Label m_sms_count_label;
  bool m_sms_count_over;
  bool m_sms_enabled, m_online;
  bool m_display_times;
  
  VPaned m_pane;

  Entry m_status;

  MessageEvent::MessageType m_message_type;

  void send_button_update();
  void set_contact_title();
  string format_time(time_t t);
  void display_message(MessageEvent *ev, bool sent, const string& nick);

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

  void setDisplayTimes(bool d);
  
  void raise() const;

  // signals
  SigC::Signal1<void,MessageEvent *> send_event;

  void send_clicked_cb();
  void switch_page_cb(Gtk::Notebook_Helpers::Page* p, guint n);
  void sms_count_update_cb();
  gint key_press_cb(GdkEventKey*);
  virtual gint delete_event_impl(GdkEventAny *ev);
};

#endif
