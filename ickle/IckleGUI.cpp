/*
 * IckleGUI
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

#include "IckleGUI.h"

#include "SettingsDialog.h"

IckleGUI::IckleGUI()
  : m_top_vbox(false),
    m_contact_scroll(),
    m_contact_list(),
    m_status(STATUS_OFFLINE)
{
  // setup default compiled in xpms
  Icons::setDefaultIcons();

  // setup callbacks
  icqclient.messaged.connect(slot(this,&IckleGUI::message_cb));
  icqclient.contactlist.connect(slot(this,&IckleGUI::contactlist_cb));
  icqclient.statuschanged.connect(slot(this,&IckleGUI::status_change_cb));

  Gtk::HButtonBox::set_child_size_default(80,30);
  Gtk::HButtonBox::set_layout_default(GTK_BUTTONBOX_SPREAD);

  set_title("ickle");
  set_wmclass( "ickle_main", "ickle_main" );
  set_border_width(5);

  m_contact_scroll.set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  m_contact_scroll.add(m_contact_list);

  m_top_vbox.pack_start(m_contact_scroll,true);

  menu_status_update();

  {
    using namespace Gtk::Menu_Helpers;

    MenuList& ml = m_ickle_menu.items();
    ml.push_back( MenuElem("Add User", slot(this, &IckleGUI::add_user_cb)) );
    ml.push_back( MenuElem("Add Mobile User", slot(this, &IckleGUI::add_mobile_user_cb)) );
    ml.push_back( MenuElem("Settings", slot(this, &IckleGUI::settings_cb)) );
    ml.push_back( MenuElem("Exit", destroy.slot()) );
    
    MenuList& mbl = m_ickle_menubar.items();
    mbl.push_front(MenuElem("Offline",m_status_menu));
    mbl.front()->right_justify();
    mbl.push_front(MenuElem("ickle",m_ickle_menu));
  }
  
  m_top_vbox.pack_end(m_ickle_menubar,false);
  
  add(m_top_vbox);

  m_contact_list.setupAccelerators();
}

void IckleGUI::menu_status_update() {
  using namespace Gtk::Menu_Helpers;
  
  MenuList& sl = m_status_menu.items();
  sl.clear();
  sl.push_back(* menu_status_widget( STATUS_ONLINE ) );
  sl.push_back(* menu_status_widget( STATUS_AWAY ) );
  sl.push_back(* menu_status_widget( STATUS_NA ) );
  sl.push_back(* menu_status_widget( STATUS_DND ) );
  sl.push_back(* menu_status_widget( STATUS_OCCUPIED ) );
  sl.push_back(* menu_status_widget( STATUS_FREEFORCHAT ) );
  sl.push_back(* menu_status_widget( STATUS_OFFLINE ) );
  m_status_menu.show_all();

}

Gtk::MenuItem* IckleGUI::menu_status_widget( Status s ) {
  Gtk::MenuItem *mi = manage( new Gtk::MenuItem() );
  mi->activate.connect( bind(slot(this,&IckleGUI::status_change_menu_cb),s) );
  Gtk::HBox *hbox=manage( new Gtk::HBox() );
  Gtk::ImageLoader *p = Icons::IconForStatus(s, false);
  hbox->pack_end( * manage( new Gtk::Pixmap(p->pix(),p->bit()) ), false, false, 3 );
  hbox->pack_end( * manage( new Gtk::Label( Status_text[s], 1.0 ) ), true );
  mi->add(*hbox);
  return mi;
}

IckleGUI::~IckleGUI() {
  while(!m_message_boxes.empty()) {
    hash_map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
    (*i).second->destroy();
  }
}

bool IckleGUI::message_cb(MessageEvent *ev) {
  Contact *c = ev->getContact();
  /*
  /* FIXME - make properly handling of incoming
  /* authorization requests
  */
  if (ev->getType() == MessageEvent::AuthReq) {     
    AuthReqEvent *msg = static_cast<AuthReqEvent*>(ev);
    AuthAckEvent ack(c, true);
    icqclient.SendEvent( &ack );
  }
  /*
   * Call the callback in the MessageBox
   * we do it this way so we don't have loads of
   * MessageBoxes listening to all the Message events
   * pointlessly
   */
  if (m_message_boxes.count(c->getUIN()) == 0) {
    return m_contact_list.message_cb(ev);
  } else {
    return m_message_boxes[c->getUIN()]->message_cb(ev);
  }

}

void IckleGUI::contactlist_cb(ContactListEvent *ev) {
  Contact *c = ev->getContact();
  unsigned int uin = c->getUIN();
  ContactListEvent::EventType et = ev->getType();

  if (m_message_boxes.count(uin) != 0) {
    MessageBox *m = m_message_boxes[uin];

    if (et == ContactListEvent::UserInfoChange) {
      m->contactlist_cb(ev);
    } else if (et == ContactListEvent::UserRemoved) {
      m_message_boxes.erase(uin);
      delete m;
    }
      
  }

  if (et == ContactListEvent::UserInfoChange && m_userinfodialog.get() != NULL) {
    m_userinfodialog->userinfochange_cb();
  }
}

ContactListView* IckleGUI::getContactListView() {
  return &m_contact_list;
}

void IckleGUI::user_popup(Contact *c) {
  if (m_message_boxes.count(c->getUIN()) == 0) {
    MessageBox *m = new MessageBox(c);
    manage(m);
    /*
     * gtkmm doesn't delete it on destroy event unless it's managed
     * however we don't add them to anything, so they still need to be signalled to
     * destroy themselves when the main window is closed.
     */
    m->destroy.connect(bind(slot(this,&IckleGUI::user_popup_close_cb), c->getUIN()));
    m->send_event.connect(send_event.slot());
    m_message_boxes[c->getUIN()] = m;

    if (m_status == STATUS_OFFLINE) m->offline();
    else m->online();

    m->setDisplayTimes(m_display_times);

    // signal all waiting messages
    while (c->numberPendingMessages() > 0) {
      MessageEvent *ev = c->getPendingMessage();
      m->message_cb(ev);
      c->erasePendingMessage(ev);
      icqclient.SignalMessageQueueChanged(c);
    }

  } else {
    // raise MessageBox
    MessageBox *m = m_message_boxes[c->getUIN()];
    m->raise();
  }
}

int IckleGUI::delete_event_impl(GdkEventAny*) {
  return false;
}

void IckleGUI::status_change_menu_cb(Status st) {
  icqclient.setStatus(st);
}

void IckleGUI::status_change_cb(MyStatusChangeEvent *ev) {
  Status st = ev->getStatus();
  Gtk::MenuItem *m = m_ickle_menubar.items()[1];
  m->remove();
  m->add_label( string(Status_text[st]) );

  m_status = st;

  hash_map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
  while (i != m_message_boxes.end()) {
    if (st == STATUS_OFFLINE) (*i).second->offline();
    else (*i).second->online();
    ++i;
  }
}

void IckleGUI::user_popup_close_cb(unsigned int uin) {
  if (m_message_boxes.count(uin) != 0) m_message_boxes.erase(uin);
}

void IckleGUI::add_user_cb() {
  AddUserDialog dialog;
  unsigned int uid = dialog.run();
  if (uid != 0) {
    add_user.emit(uid);
  }
}

void IckleGUI::add_mobile_user_cb() {
  AddMobileUserDialog dialog;
  if (dialog.run()) {
    add_mobile_user.emit( dialog.getAlias(), dialog.getMobileNo() );
  }
}

void IckleGUI::invalid_login_prompt() {
  PromptDialog pd(PromptDialog::PROMPT_WARNING, "You have not entered a valid UIN and Password. "
	                                     "Go to Settings and enter the details.");
  pd.run();
}

void IckleGUI::setDisplayTimes(bool d) {
  if (m_display_times != d) {
    m_display_times = d;
    
    hash_map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
    while (i != m_message_boxes.end()) {
      (*i).second->setDisplayTimes(d);
      ++i;
    }

  }
}

void IckleGUI::user_info_edit(Contact *c) {
  m_userinfodialog.reset(new UserInfoDialog(c));
  m_userinfodialog->fetch.connect( bind( fetch.slot(), c) );
  if (m_userinfodialog->run()) icqclient.SignalUserInfoChange(c);
  m_userinfodialog.reset();
}

void IckleGUI::settings_cb() {
  SettingsDialog dialog;
  dialog.icons_changed.connect( slot( this, &IckleGUI::icons_changed_cb ) );

  if (dialog.run()) {
    bool reconnect = false;
    if (dialog.getUIN() != icqclient.getUIN() ||
	dialog.getPassword() != icqclient.getPassword()) reconnect = icqclient.isConnected();

    if (reconnect) icqclient.Disconnect();
    
    dialog.updateSettings();

    if (dialog.getUIN() != icqclient.getUIN()) icqclient.setUIN(dialog.getUIN());
    if (dialog.getPassword() != icqclient.getPassword()) icqclient.setPassword(dialog.getPassword());
  
    if (reconnect) status_change_menu_cb(STATUS_ONLINE);

    settings_changed.emit();
  }

}

void IckleGUI::icons_changed_cb(string s) {
  Icons::setIcons(s);
  menu_status_update();
  m_contact_list.icons_changed_cb();
}
