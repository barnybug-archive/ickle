/* $Id: IckleGUI.h,v 1.17 2001-12-18 19:45:10 nordman Exp $
 * 
 * The 'looks' part of Ickle (the view)
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

#ifndef ICKLEGUI_H
#define ICKLEGUI_H

#include <gtk--/window.h>
#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/menu.h>
#include <gtk--/menubar.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/dialog.h>

#include <sigc++/signal_system.h>

#include <string>

#include <config.h>

#ifdef HAVE_EXT_HASH_MAP
# include <ext/hash_map>
#elif HAVE_HASH_MAP
# include <hash_map>
#else
# error "hash_map not defined"
#endif

#include <utility>

#include "main.h"
#include "Client.h"

#include "ContactList.h"
#include "Contact.h"

#include "events.h"
#include "ContactListView.h"
#include "MessageBox.h"
#include "AddUserDialog.h"
#include "AddMobileUserDialog.h"
#include "UserInfoDialog.h"
#include "PromptDialog.h"
#include "AwayMessageDialog.h"
#include "History.h"

#include "constants.h"

using std::string;
using std::hash_map;

using SigC::Signal0;
using SigC::Signal1;
using SigC::Signal2;

using namespace ICQ2000;

class IckleGUI : public Gtk::Window {
 private:
  hash_map<unsigned int, MessageBox*> m_message_boxes;
  hash_map<unsigned int, UserInfoDialog*> m_userinfo_dialogs;
  Status m_status;

  bool m_display_times;

  // gtk widgets
  Gtk::VBox m_top_vbox;
  Gtk::ScrolledWindow  m_contact_scroll;
  ContactListView m_contact_list;
  AwayMessageDialog m_away_message;
  
  Gtk::MenuBar m_ickle_menubar;
  Gtk::Menu m_ickle_menu;
  Gtk::Menu m_status_menu;

  // --

  void menu_status_update();
  Gtk::MenuItem* menu_status_widget( Status s );

  void messagebox_popup(Contact *c, History *h);

 public:
  IckleGUI();
  ~IckleGUI();

  ContactListView* getContactListView();

  void status_change_menu_cb(Status st);
  void popup_messagebox(Contact *c, History *h);
  void userinfo_popup(Contact *c);
  void message_box_close_cb(Contact *c);
  void userinfo_dialog_close_cb(Contact *c);
  void add_user_cb();
  void add_mobile_user_cb();
  void invalid_login_prompt();

  void setDisplayTimes(bool d);

  gint ickle_popup_cb(GdkEventButton*);

  // callbacks
  void contactlist_cb(ContactListEvent* ev);
  bool message_cb(MessageEvent* ev);
  void messageack_cb(MessageEvent* ev);
  void settings_cb();
  void settings_changed_cb(const string &key);
  void userinfo_toggle_cb(bool b, Contact *c);
  void status_change_cb(MyStatusChangeEvent *ev);

  // signals
  Signal0<void> settings_changed;
  Signal1<void,Status> status_changed;
  Signal1<void,MessageEvent*> send_event;
  Signal1<void,unsigned int> add_user;
  Signal2<void,string,string> add_mobile_user;
  Signal1<void,Contact*> fetch;
  Signal1<void,unsigned int> user_popup;

  // handle wm calls
  gint delete_event_impl(GdkEventAny*);
};

#endif
