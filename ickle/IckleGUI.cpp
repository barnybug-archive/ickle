/* $Id: IckleGUI.cpp,v 1.25 2001-12-27 15:16:18 nordman Exp $
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

#include "IckleGUI.h"

#include "SettingsDialog.h"
#include "sstream_fix.h"

using std::map;

IckleGUI::IckleGUI()
  : m_top_vbox(false),
    m_contact_scroll(),
    m_contact_list(),
    m_status(STATUS_OFFLINE),
    m_status_wanted(STATUS_OFFLINE),
    m_invisible(false),
    m_invisible_wanted(false),
    m_away_message( this )
{
  // setup default compiled in xpms
  g_icons.setDefaultIcons();

  // setup callbacks
  icqclient.messaged.connect(slot(this,&IckleGUI::message_cb));
  icqclient.messageack.connect(slot(this,&IckleGUI::messageack_cb));
  icqclient.contactlist.connect(slot(this,&IckleGUI::contactlist_cb));
  icqclient.statuschanged.connect(slot(this,&IckleGUI::status_change_cb));

  g_icons.icons_changed.connect( slot( this, &IckleGUI::icons_changed_cb ) );

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
    ml.push_back( MenuElem("Exit", slot(this, &IckleGUI::exit_cb)) );
    
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
  sl.push_back( SeparatorElem() );
  sl.push_back(* menu_status_inv_widget() );
  m_status_menu.show_all();

}

Gtk::MenuItem* IckleGUI::menu_status_widget( Status s ) {
  Gtk::MenuItem *mi;
  mi = manage( new Gtk::MenuItem() );
  mi->activate.connect( bind(slot(this,&IckleGUI::status_change_menu_cb),s) );
  Gtk::HBox *hbox=manage( new Gtk::HBox() );
  Gtk::ImageLoader *p = g_icons.IconForStatus(s, false);
  hbox->pack_end( * manage( new Gtk::Pixmap(p->pix(),p->bit()) ), false, false, 3 );
  hbox->pack_end( * manage( new Gtk::Label( Status_text[s], 1.0 ) ), true );
  mi->add(*hbox);
  return mi;
}
  
Gtk::MenuItem* IckleGUI::menu_status_inv_widget() {
  Gtk::CheckMenuItem *mi;
  mi = manage( new Gtk::CheckMenuItem() );
  mi->toggled.connect( bind(slot(this,&IckleGUI::status_change_inv_menu_cb), mi) );
  Gtk::HBox *hbox=manage( new Gtk::HBox() );
  Gtk::ImageLoader *p = g_icons.IconForStatus(STATUS_ONLINE, true);
  hbox->pack_end( * manage( new Gtk::Pixmap(p->pix(),p->bit()) ), false, false, 3 );
  hbox->pack_end( * manage( new Gtk::Label( "Invisible", 1.0 ) ), true );
  mi->add(*hbox);
  return mi;
}
  
IckleGUI::~IckleGUI() {
  while(!m_message_boxes.empty()) {
    map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
    (*i).second->destroy();
  }
  while(!m_userinfo_dialogs.empty()) {
    map<unsigned int, UserInfoDialog*>::iterator i = m_userinfo_dialogs.begin();
    (*i).second->destroy();
  }
}

bool IckleGUI::message_cb(MessageEvent *ev) {
  Contact *c = ev->getContact();
  /*
   * FIXME - make properly handling of incoming
   * authorization requests
   */
  if (ev->getType() == MessageEvent::AuthReq) {     
    AuthReqEvent *msg = static_cast<AuthReqEvent*>(ev);
    AuthAckEvent *ack = new AuthAckEvent(c, true);
    icqclient.SendEvent( ack );
  }


  if (m_message_boxes.count(c->getUIN()) == 0) {
    if ( g_settings.getValueBool("message_autopopup") ) {
      user_popup.emit( c->getUIN() ); // popup a new messagebox
      return true;
    } else {
      return m_contact_list.message_cb(ev);
    }
  } else {
    if ( g_settings.getValueBool("message_autoraise") ) {
      user_popup.emit( c->getUIN() );  // raise existing messagebox
      return true; // history now handles signalling new messages to open messageboxes
    }
  }

  return false;
}

void IckleGUI::messageack_cb(MessageEvent *ev) {
  Contact *c = ev->getContact();

  if (m_message_boxes.count(c->getUIN()) > 0) {
    m_message_boxes[c->getUIN()]->messageack_cb(ev);
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
      m->destroy();
    }
      
  }

  if (m_userinfo_dialogs.count(uin) != 0) {
    UserInfoDialog *d = m_userinfo_dialogs[uin];

    if (et == ContactListEvent::UserInfoChange) {
      d->userinfochange_cb();
    } else if (et == ContactListEvent::UserRemoved) {
      d->destroy();
    }
      
  }

}

ContactListView* IckleGUI::getContactListView() {
  return &m_contact_list;
}

void IckleGUI::messagebox_popup(Contact *c, History *h) {
  if (m_message_boxes.count(c->getUIN()) == 0) {
    MessageBox *m = new MessageBox(c, h);
    manage(m);
    /*
     * gtkmm doesn't delete it on destroy event unless it's managed
     * however we don't add them to anything, so they still need to be signalled to
     * destroy themselves when the main window is closed.
     */
    m->destroy.connect(bind(slot(this,&IckleGUI::message_box_close_cb), c));
    m->send_event.connect(send_event.slot());
    m->userinfo_dialog.connect(bind(slot(this,&IckleGUI::userinfo_toggle_cb), c));
    m_message_boxes[c->getUIN()] = m;

    if (m_status == STATUS_OFFLINE) m->offline();
    else m->online();

    if (m_userinfo_dialogs.count(c->getUIN()) > 0) m->userinfo_dialog_cb(true);

    m->setDisplayTimes(m_display_times);
    m->popup();
  } else {
    // raise MessageBox
    MessageBox *m = m_message_boxes[c->getUIN()];
    m->raise();
  }
}

void IckleGUI::popup_messagebox(Contact *c, History *h) {
  messagebox_popup(c,h);

  if (m_message_boxes.count(c->getUIN()) > 0) {
    MessageBox *m = m_message_boxes[c->getUIN()];

    // signal all waiting messages
    while (c->numberPendingMessages() > 0) {
      MessageEvent *ev = c->getPendingMessage();
      c->erasePendingMessage(ev);
    }
    icqclient.SignalMessageQueueChanged(c);
  }
}

int IckleGUI::delete_event_impl(GdkEventAny*) {
#ifdef GNOME_ICKLE
  hide();
  return true;
#else
  return false;
#endif
}

void IckleGUI::status_change_menu_cb(Status st) {
  m_status_wanted = st;
  if (st != STATUS_ONLINE && st != STATUS_OFFLINE) {
    SetAutoResponseDialog *d = new SetAutoResponseDialog(auto_response);
    manage(d);
    d->save_new_msg.connect(slot(this, &IckleGUI::setAutoResponse));
  }
  icqclient.setStatus(m_status_wanted, m_invisible_wanted);
}

void IckleGUI::status_change_inv_menu_cb(Gtk::CheckMenuItem *cmi) 
{
  m_invisible_wanted = cmi->is_active();
  icqclient.setStatus(m_status_wanted, m_invisible_wanted);
}
 
void IckleGUI::status_change_cb(MyStatusChangeEvent *ev) {
  Status st = ev->getStatus();
  Gtk::MenuItem *m = m_ickle_menubar.items()[1];
  m->remove();
  ostringstream ostr;
  ostr << Status_text[st];
  if (ev->getInvisible()) ostr << " (I)";
  m->add_label( ostr.str() );

  m_status = st;
  m_invisible = ev->getInvisible();

  map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
  while (i != m_message_boxes.end()) {
    if (st == STATUS_OFFLINE) (*i).second->offline();
    else (*i).second->online();
    ++i;
  }
}

void IckleGUI::message_box_close_cb(Contact *c) {
  unsigned int uin = c->getUIN();
  if (m_message_boxes.count(uin) != 0) m_message_boxes.erase(uin);
  if (m_userinfo_dialogs.count(uin) != 0) m_userinfo_dialogs[uin]->destroy();
}

void IckleGUI::userinfo_dialog_close_cb(Contact *c) {
  unsigned int uin = c->getUIN();
  if (m_userinfo_dialogs.count(uin) != 0) {
    if (m_userinfo_dialogs[uin]->isChanged()) icqclient.SignalUserInfoChange(c);
    m_userinfo_dialogs.erase(uin);
  }

  if (m_message_boxes.count(uin) != 0) {
    m_message_boxes[uin]->userinfo_dialog_cb(false);
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

void IckleGUI::turboing_prompt(){
  PromptDialog pd(PromptDialog::PROMPT_WARNING, "The server says you have been turboing. "
		  "This means you've been connecting and disconnecting to the server too "
		  "quickly and so it has blocked you temporarily. Don't be alarmed, just "
		  "wait for at least 5 minutes before reattempting. If you still get this "
		  "message then wait a bit longer - maybe up to an hour.");
  pd.run();
}

void IckleGUI::setDisplayTimes(bool d) {
  if (m_display_times != d) {
    m_display_times = d;
    
    map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
    while (i != m_message_boxes.end()) {
      (*i).second->setDisplayTimes(d);
      ++i;
    }

  }
}

void IckleGUI::setAutoResponse(const string& ar) {
  auto_response = ar;
  g_settings.setValue("last_auto_response", ar);
}

string IckleGUI::getAutoResponse() const
{
  return auto_response;
}

void IckleGUI::userinfo_toggle_cb(bool b, Contact *c) {
  unsigned int uin = c->getUIN();
  if ( b && m_userinfo_dialogs.count(uin) == 0 ) {
    userinfo_popup(c);
  } else if ( !b && m_userinfo_dialogs.count(uin) > 0 ) {
    m_userinfo_dialogs[uin]->destroy();
  }
}

void IckleGUI::userinfo_popup(Contact *c) {
  unsigned int uin = c->getUIN();
  if ( m_userinfo_dialogs.count(uin) == 0 ) {
    UserInfoDialog *d = new UserInfoDialog(c);
    manage(d);
    d->destroy.connect(bind(slot(this,&IckleGUI::userinfo_dialog_close_cb), c));
    d->fetch.connect( bind( fetch.slot(), c) );
    m_userinfo_dialogs[ uin ] = d;
    if (m_message_boxes.count(uin) != 0) {
      m_message_boxes[uin]->userinfo_dialog_cb(true);
    }
  } else {
    m_userinfo_dialogs[ uin ]->raise();
  }

}

void IckleGUI::settings_cb() {
  SettingsDialog dialog;

  if (dialog.run()) {
    bool reconnect = false;
    if (dialog.getUIN() != icqclient.getUIN() ||
	dialog.getPassword() != icqclient.getPassword()) reconnect = icqclient.isConnected();

    if (reconnect) icqclient.setStatus(STATUS_OFFLINE);
    
    dialog.updateSettings();

    if (dialog.getUIN() != icqclient.getUIN()) icqclient.setUIN(dialog.getUIN());
    if (dialog.getPassword() != icqclient.getPassword()) icqclient.setPassword(dialog.getPassword());
  
    if (reconnect) status_change_menu_cb(STATUS_ONLINE);

    settings_changed.emit();
  }

}

void IckleGUI::icons_changed_cb() {
  menu_status_update();
  m_contact_list.icons_changed_cb();
}

void IckleGUI::exit_cb() {
  exit.emit ();
  destroy ();
}
