/* $Id: IckleGUI.h,v 1.40 2002-04-19 15:46:23 bugcreator Exp $
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
#include "UserInfoDialog.h"
#include "PromptDialog.h"
#include "AwayMessageDialog.h"
#include "History.h"
#include "SetAutoResponseDialog.h"
#include "StatusMenu.h"
#include "MessageQueue.h"

class IckleGUI : public Gtk::Window {
 private:
  MessageQueue& m_message_queue;

  std::map<unsigned int, MessageBox*> m_message_boxes;
  std::map<unsigned int, UserInfoDialog*> m_userinfo_dialogs;
  ICQ2000::Status m_status;
  bool m_invisible;
  std::string auto_response;

  bool m_display_times;

  // gtk widgets
  Gtk::VBox m_top_vbox;
  Gtk::ScrolledWindow  m_contact_scroll;
  ContactListView m_contact_list;
  AwayMessageDialog m_away_message;
  
  Gtk::MenuBar m_ickle_menubar;
  Gtk::Menu m_ickle_menu;
  StatusMenu m_status_menu;
  Gtk::MenuItem *mi_search_for_contacts;

  int geometry_x, geometry_y;
  int geometry_w, geometry_h;

  // --

  void create_messagebox(const ICQ2000::ContactRef& c, History *h);
  void raise_messagebox(const ICQ2000::ContactRef& c);

  gint remove_from_queue_idle_cb(MessageEvent *ev);
  void remove_from_queue_delayed(MessageEvent *ev);

  void set_ickle_title();
  bool m_exiting;

 public:
  IckleGUI(MessageQueue& mq);
  ~IckleGUI();

  ContactListView* getContactListView();

  void status_menu_status_changed_cb(ICQ2000::Status st);
  void status_menu_invisible_changed_cb(bool inv);
  void status_menu_status_inv_changed_cb(ICQ2000::Status st, bool inv);

  void set_auto_response_dialog (bool timeout);

  void popup_next_event(const ICQ2000::ContactRef& c, History *h);
  void popup_messagebox(const ICQ2000::ContactRef& c, History *h);
  void popup_auth_req(const ICQ2000::ContactRef& c, AuthReqICQMessageEvent *ev);
  void popup_auth_resp(const ICQ2000::ContactRef& c, AuthAckICQMessageEvent *ev);
  void popup_user_added_you(const ICQ2000::ContactRef& c, UserAddICQMessageEvent *ev);
  void popup_userinfo(const ICQ2000::ContactRef& c);

  // important - this are not passed by reference, as otherwise ref
  // counting screws up when used inconjunction with SigC::bind
  void message_box_close_cb(ICQ2000::ContactRef c);
  void userinfo_dialog_close_cb(ICQ2000::ContactRef c);
  void userinfo_dialog_upload_cb(ICQ2000::ContactRef c);
  
  // -- menu callbacks --
  void add_user_cb();
  void search_user_cb();
  void about_cb();
  void my_user_info_cb();

  // -- prompts --
  void invalid_login_prompt();
  void turboing_prompt();
  void duallogin_prompt();
  void disconnect_lowlevel_prompt(int retries);
  void disconnect_unknown_prompt(int retries);
  void already_running_prompt(const std::string& pid_file, unsigned int pid);

  void setDisplayTimes(bool d);
  string getAutoResponse() const;
  void setAutoResponse(const std::string& ar);
  void setGeometry(int x, int y, int w, int h);

  gint ickle_popup_cb(GdkEventButton*);

  // -- library callbacks --
  void contactlist_cb(ICQ2000::ContactListEvent* ev);
  void messageack_cb(ICQ2000::MessageEvent* ev);
  void self_status_change_cb(ICQ2000::StatusChangeEvent *ev);
  void self_userinfo_change_cb(ICQ2000::UserInfoChangeEvent *ev);

  // -- MessageQueue callbacks --
  void queue_added_cb(MessageEvent *ev);
  void queue_removed_cb(MessageEvent *ev);

  // -- other callbacks --
  void settings_cb(Gtk::Window * away_dlg);
  void icons_changed_cb();
  void userinfo_toggle_cb(bool b, ICQ2000::ContactRef c);
  void exit_cb();
  void userinfo_fetch_cb(ICQ2000::ContactRef c);
  void my_userinfo_fetch_cb();
  void settings_changed_cb(const std::string& k);

  void spell_check_setup();

  // signals
  SigC::Signal0<void> settings_changed;
  SigC::Signal1<void,ICQ2000::MessageEvent*> send_event;
  SigC::Signal1<void,unsigned int> user_popup;
  SigC::Signal0<void> exit;

  // handle wm calls
  void show_impl();
  void hide_impl();
  gint delete_event_impl(GdkEventAny*);
};

#endif
