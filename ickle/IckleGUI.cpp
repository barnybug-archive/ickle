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

IckleGUI::IckleGUI()
  : m_top_vbox(false),
    m_contact_scroll(),
    m_contact_list(),
    m_status(STATUS_OFFLINE)
{

  // setup callbacks
  icqclient.messaged.connect(slot(this,&IckleGUI::message_cb));
  icqclient.contactlist.connect(slot(this,&IckleGUI::contactlist_cb));

  Gtk::HButtonBox::set_child_size_default(80,30);
  Gtk::HButtonBox::set_layout_default(GTK_BUTTONBOX_SPREAD);

  set_title("ickle");
  set_border_width(5);
  set_usize(130,300);

  m_contact_scroll.set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  m_contact_scroll.add(m_contact_list);

  m_top_vbox.pack_start(m_contact_scroll,true);

  {
    using namespace Menu_Helpers;

    MenuList& sl = m_status_menu.items();
    sl.push_back(MenuElem("Online",bind(slot(this,&IckleGUI::status_change_cb),STATUS_ONLINE)));
    sl.push_back(MenuElem("Away",bind(slot(this,&IckleGUI::status_change_cb),STATUS_AWAY)));
    sl.push_back(MenuElem("N/A",bind(slot(this,&IckleGUI::status_change_cb),STATUS_NA)));
    sl.push_back(MenuElem("DND",bind(slot(this,&IckleGUI::status_change_cb),STATUS_DND)));
    sl.push_back(MenuElem("Occupied",bind(slot(this,&IckleGUI::status_change_cb),STATUS_OCCUPIED)));
    sl.push_back(MenuElem("Free for chat",bind(slot(this,&IckleGUI::status_change_cb),STATUS_FREEFORCHAT)));
    sl.push_back(MenuElem("Offline",bind(slot(this,&IckleGUI::status_change_cb),STATUS_OFFLINE)));

    MenuList& ml = m_ickle_menu.items();
    ml.push_back( MenuElem("Add User", slot(this, &IckleGUI::add_user_cb)) );
    ml.push_back( MenuElem("Add Mobile User", slot(this, &IckleGUI::add_mobile_user_cb)) );
    ml.push_back( MenuElem("Settings", settings.slot()) );
    ml.push_back( MenuElem("Exit", destroy.slot()) );
    
    MenuList& mbl = m_ickle_menubar.items();
    mbl.push_front(MenuElem("Offline",m_status_menu));
    mbl.front()->right_justify();
    mbl.push_front(MenuElem("ickle",m_ickle_menu));
  }

  m_top_vbox.pack_end(m_ickle_menubar,false);
  
  add(m_top_vbox);
  show_all();
}

IckleGUI::~IckleGUI() {
  while(!m_message_boxes.empty()) {
    hash_map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
    delete ((*i).second);
    m_message_boxes.erase(i);
  }
}

bool IckleGUI::message_cb(MessageEvent *ev) {
  Contact *c = ev->getContact();
  
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
}

ContactListView* IckleGUI::getContactListView() {
  return &m_contact_list;
}

void IckleGUI::user_popup(Contact *c) {
  if (m_message_boxes.count(c->getUIN()) == 0) {
    MessageBox *m = new MessageBox(c);
    m->close.connect(bind(slot(this,&IckleGUI::user_popup_close_cb), c->getUIN()));
    m->send_event.connect(send_event.slot());
    m_message_boxes[c->getUIN()] = m;

    if (m_status == STATUS_OFFLINE) m->offline();
    else m->online();

    m->setDisplayTimes(d);

    // signal all waiting messages
    while (c->numberPendingMessages() > 0) {
      MessageEvent *ev = c->getPendingMessage();
      m->message_cb(ev);
      c->erasePendingMessage(ev);
      icqclient.SignalMessageQueueChanged(c);
    }

  } else {
    // maybe bring to front?
  }
}

void IckleGUI::setStatus(Status st) {
  MenuItem *m = m_ickle_menubar.items()[1];
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

int IckleGUI::delete_event_impl(GdkEventAny*) {
  return false;
}

void IckleGUI::status_change_cb(Status st) {
  status_changed.emit(st);
}

void IckleGUI::user_popup_close_cb(unsigned int uin) {
  if (m_message_boxes.count(uin) != 0) {
    MessageBox *m = m_message_boxes[uin];
    m_message_boxes.erase(uin);
    delete m;
  }
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
  UserInfoDialog dialog(c);
  if (dialog.run()) icqclient.SignalUserInfoChange(c);
}
