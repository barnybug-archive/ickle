/* $Id: IckleGUI.h,v 1.28 2002-01-21 13:30:35 barnabygray Exp $
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
#include <map>
#include <utility>

#include <libicq2000/ContactList.h>
#include <libicq2000/Contact.h>
#include <libicq2000/events.h>

#include <libicq2000/constants.h>

#include "ContactListView.h"
#include "MessageBox.h"
#include "AddUserDialog.h"
#include "AddMobileUserDialog.h"
#include "UserInfoDialog.h"
#include "PromptDialog.h"
#include "AwayMessageDialog.h"
#include "History.h"
#include "SetAutoResponseDialog.h"
#include "StatusMenu.h"

using std::string;
using std::map;

using namespace ICQ2000;

class IckleGUI : public Gtk::Window {
 private:
  std::map<unsigned int, MessageBox*> m_message_boxes;
  std::map<unsigned int, UserInfoDialog*> m_userinfo_dialogs;
  Status m_status, m_status_wanted;
  bool m_invisible, m_invisible_wanted;
  string auto_response;

  bool m_display_times;

  // gtk widgets
  Gtk::VBox m_top_vbox;
  Gtk::ScrolledWindow  m_contact_scroll;
  ContactListView m_contact_list;
  AwayMessageDialog m_away_message;
  
  Gtk::MenuBar m_ickle_menubar;
  Gtk::Menu m_ickle_menu;
  StatusMenu m_status_menu;

  // --

  void messagebox_popup(Contact *c, History *h);

 public:
  IckleGUI();
  ~IckleGUI();

  ContactListView* getContactListView();

  void status_menu_status_changed_cb(Status st);
  void status_menu_invisible_changed_cb(bool inv);

  void popup_messagebox(Contact *c, History *h);
  void userinfo_popup(Contact *c);
  void message_box_close_cb(Contact *c);
  void userinfo_dialog_close_cb(Contact *c);
  void userinfo_dialog_upload_cb(Contact *c);
  
  // -- menu callbacks --
  void add_user_cb();
  void add_mobile_user_cb();
  void search_user_cb();
  void about_cb();
  void my_user_info_cb();

  // -- disconnected callbacks --
  void invalid_login_prompt();
  void turboing_prompt();

  void setDisplayTimes(bool d);
  string getAutoResponse() const;
  void setAutoResponse(const string& ar);

  gint ickle_popup_cb(GdkEventButton*);

  // -- library callbacks --
  void contactlist_cb(ContactListEvent* ev);
  bool message_cb(MessageEvent* ev);
  void messageack_cb(MessageEvent* ev);
  void self_event_cb(SelfEvent *ev);

  // -- other callbacks --
  void settings_cb();
  void userinfo_toggle_cb(bool b, Contact *c);
  void exit_cb();
  void userinfo_fetch_cb(Contact *c);
  void my_userinfo_fetch_cb();
  void settings_changed_cb(const string& k);

  void spell_check_setup();

  // signals
  SigC::Signal0<void> settings_changed;
  SigC::Signal1<void,Status> status_changed;
  SigC::Signal1<void,MessageEvent*> send_event;
  SigC::Signal1<void,unsigned int> add_user;
  SigC::Signal2<void,string,string> add_mobile_user;
  SigC::Signal1<void,unsigned int> user_popup;
  SigC::Signal0<void> exit;

  // handle wm calls
  void show_impl();
  void hide_impl();
  gint delete_event_impl(GdkEventAny*);
};

#endif
