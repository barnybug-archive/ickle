/* $Id: IckleGUI.cpp,v 1.36 2002-01-30 15:35:35 nordman Exp $
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
#include "SearchDialog.h"
#include "AboutDialog.h"
#include "PromptDialog.h"

#include <libicq2000/Client.h>

#include "main.h"

#include "sstream_fix.h"

#include "gtkspell.h"

using std::map;
using std::ostringstream;

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
  // setup callbacks
  icqclient.messaged.connect(slot(this,&IckleGUI::message_cb));
  icqclient.messageack.connect(slot(this,&IckleGUI::messageack_cb));
  icqclient.contactlist.connect(slot(this,&IckleGUI::contactlist_cb));
  icqclient.self_event.connect(slot(this,&IckleGUI::self_event_cb));

  Gtk::HButtonBox::set_child_size_default(80,30);
  Gtk::HButtonBox::set_layout_default(GTK_BUTTONBOX_SPREAD);

  set_title("ickle");
  set_wmclass( "ickle_main", "ickle_main" );
  set_border_width(5);

  m_contact_scroll.set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  m_contact_scroll.add(m_contact_list);

  m_top_vbox.pack_start(m_contact_scroll,true);

  {
    using namespace Gtk::Menu_Helpers;

    MenuList& ml = m_ickle_menu.items();
    ml.push_back( MenuElem("Add Contact", slot(this, &IckleGUI::add_user_cb)) );
    ml.push_back( MenuElem("Add Mobile Contact", slot(this, &IckleGUI::add_mobile_user_cb)) );
    ml.push_back( MenuElem("My User Info", slot(this, &IckleGUI::my_user_info_cb)) );
    ml.push_back( MenuElem("Search for Contacts", slot(this, &IckleGUI::search_user_cb)) );
    mi_search_for_contacts = ml.back();
    mi_search_for_contacts->set_sensitive(false);
    ml.push_back( MenuElem("Settings", slot(this, &IckleGUI::settings_cb)) );
    ml.push_back( MenuElem("About", slot(this, &IckleGUI::about_cb)) );
    ml.push_back( MenuElem("Exit", slot(this, &IckleGUI::exit_cb)) );
    
    MenuList& mbl = m_ickle_menubar.items();
    mbl.push_front( m_status_menu );
    m_status_menu.status_changed_status.connect( slot( this, &IckleGUI::status_menu_status_changed_cb ) );
    m_status_menu.status_changed_invisible.connect( slot( this, &IckleGUI::status_menu_invisible_changed_cb ) );
    
    mbl.front()->right_justify();
    mbl.push_front(MenuElem("ickle",m_ickle_menu));
  }
  
  m_top_vbox.pack_end(m_ickle_menubar,false);
  
  add(m_top_vbox);

  m_contact_list.setupAccelerators();

  g_settings.settings_changed.connect( slot( this, &IckleGUI::settings_changed_cb ) );
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

  if (gtkspell_running())
    gtkspell_stop();
  
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
    Contact *self = icqclient.getSelfContact();
    MessageBox *m = new MessageBox(self, c, h);
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

    if (gtkspell_running())
      m->spell_attach();

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

void IckleGUI::setGeometry(int x, int y)
{
  geometry_x = x;
  geometry_y = y;
  set_uposition(geometry_x, geometry_y);
}

void IckleGUI::show_impl() {
  set_uposition(geometry_x, geometry_y);
  Window::show_impl();
}

void IckleGUI::hide_impl() {
  get_window().get_root_origin(geometry_x, geometry_y);
  Window::hide_impl();
}

int IckleGUI::delete_event_impl(GdkEventAny*) {
#ifdef GNOME_ICKLE
  hide();
  return true;
#else
  return false;
#endif
}

void IckleGUI::status_menu_status_changed_cb(Status st) {
  m_status_wanted = st;
  
  if (st != STATUS_ONLINE && st != STATUS_OFFLINE) {
    SetAutoResponseDialog *d = new SetAutoResponseDialog(auto_response);
    manage(d);
    d->save_new_msg.connect(slot(this, &IckleGUI::setAutoResponse));
  }
  icqclient.setStatus(m_status_wanted, m_invisible_wanted);
}

void IckleGUI::status_menu_invisible_changed_cb(bool inv)
{
  m_invisible_wanted = inv;
  icqclient.setStatus(m_status_wanted, m_invisible_wanted);
}

void IckleGUI::self_event_cb(SelfEvent *ev) {
  Contact *c = ev->getSelfContact();

  if (ev->getType() == SelfEvent::MyStatusChange) {
    MyStatusChangeEvent *mev = static_cast<MyStatusChangeEvent*>(ev);

    Status st = mev->getStatus();
    bool inv = mev->getInvisible();

    m_status_menu.status_changed_cb( st, inv );
    m_status = st;
    m_invisible = inv;
    
    map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
    while (i != m_message_boxes.end()) {
      if (st == STATUS_OFFLINE) (*i).second->offline();
      else (*i).second->online();
      ++i;
    }

    // update connection dependent menu entry
    mi_search_for_contacts->set_sensitive(st != STATUS_OFFLINE);
    
  } else if (ev->getType() == SelfEvent::MyUserInfoChange) {
    if (m_userinfo_dialogs.count(c->getUIN()) != 0) {
      UserInfoDialog *d = m_userinfo_dialogs[c->getUIN()];
      d->userinfochange_cb();
    }
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

void IckleGUI::userinfo_dialog_upload_cb(Contact *c) {
  unsigned int uin = c->getUIN();
  if (m_userinfo_dialogs.count(uin) != 0) {
    if (m_userinfo_dialogs[uin]->isChanged()) icqclient.SignalUserInfoChange(c);
    icqclient.uploadSelfDetails();
  }
}

void IckleGUI::add_user_cb() {
  AddUserDialog dialog;
  unsigned int uid = dialog.run();
  if (uid != 0) {
    add_user.emit(uid);
  }
}

void IckleGUI::search_user_cb() 
{
  SearchDialog *sd = new SearchDialog();
  manage( sd );
}

void IckleGUI::my_user_info_cb()
{
  Contact *self = icqclient.getSelfContact();
  unsigned int uin = self->getUIN();
  
  if ( m_userinfo_dialogs.count(uin) == 0 ) {
    UserInfoDialog *d = new UserInfoDialog(self, true);
    manage(d);
    d->destroy.connect(bind(slot(this,&IckleGUI::userinfo_dialog_close_cb), self));
    d->fetch.connect( slot( this, &IckleGUI::my_userinfo_fetch_cb ) );
    d->upload.connect( bind( slot( this, &IckleGUI::userinfo_dialog_upload_cb ), self));
    m_userinfo_dialogs[ uin ] = d;
    if (m_message_boxes.count(uin) != 0) {
      m_message_boxes[uin]->userinfo_dialog_cb(true);
    }
  } else {
    m_userinfo_dialogs[ uin ]->raise();
  }
}

void IckleGUI::userinfo_fetch_cb(Contact *c)
{
  icqclient.fetchDetailContactInfo(c);
}

void IckleGUI::my_userinfo_fetch_cb()
{
  icqclient.fetchSelfDetailContactInfo();
}

void IckleGUI::about_cb()
{
  AboutDialog about;
  about.run();
}

void IckleGUI::add_mobile_user_cb() {
  AddMobileUserDialog dialog;
  if (dialog.run()) {
    add_mobile_user.emit( dialog.getAlias(), dialog.getMobileNo() );
  }
}

void IckleGUI::invalid_login_prompt() {
  PromptDialog pd(PromptDialog::PROMPT_WARNING, "You have not entered a valid UIN and Password. "
	                                     "Go to Settings and correct the details.");
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

void IckleGUI::duallogin_prompt() {
  PromptDialog pd(PromptDialog::PROMPT_WARNING,
                  "The server recieved multiple simultaneous login for this account"
                  "Do you have multiple clients running with the same account?" );
  pd.run();
}

void IckleGUI::disconnect_lowlevel_prompt(int retries) {
  ostringstream os;
  os << "There occured a networking error while communicating with the server, and as a "
    "result you were disconnected.";
  if( retries ) {
    os << " Ickle tried to reconnect " << retries << " times, unfortunately with little success."
       << "You will have to manually attempt to reconnect, preferably after waiting a short while.";
  }
  
  PromptDialog pd( PromptDialog::PROMPT_WARNING, os.str() );
  pd.run();
}

void IckleGUI::disconnect_unknown_prompt(int retries) {
  ostringstream os;
  os << "There occured an unknown error while communicating with the server, and as a "
    "result you were disconnected.";
  if( retries ) {
    os << " Ickle tried to reconnect " << retries << " times, unfortunately with little success."
       << "You will have to manually attempt to reconnect, preferably after waiting a short while.";
  }
  
  PromptDialog pd( PromptDialog::PROMPT_WARNING, os.str() );
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
    d->fetch.connect( bind( slot(this, &IckleGUI::userinfo_fetch_cb), c) );
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
  
    if (reconnect) status_menu_status_changed_cb(STATUS_ONLINE);

    settings_changed.emit();
  }

}

void IckleGUI::spell_check_setup()
{
  if (g_settings.getValueBool("spell_check")) {
    // start gtkspell

    char *ispellcmd[] = { "ispell", "-a", NULL };
    char *aspellcmd[] = { "aspell", "pipe", NULL };

    char **spellcmd;
    
    if (g_settings.getValueBool("spell_check_aspell")) {
      spellcmd = aspellcmd;
    } else {
      spellcmd = ispellcmd;
    }
          
    if (gtkspell_running()) gtkspell_stop();
    
    if (gtkspell_start(NULL, spellcmd) == 0) {
      map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
      while (i != m_message_boxes.end()) {
	(*i).second->spell_attach();
	++i;
      }
    }

  } else {
    // stop gtkspell

    if (gtkspell_running()) {
      gtkspell_stop();
      map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
      while (i != m_message_boxes.end()) {
	(*i).second->spell_detach();
	++i;
      }
    }
      
  }
  
}


void IckleGUI::settings_changed_cb(const string& k)
{
  if (k == "spell_check")
    spell_check_setup();
  else if (k == "spell_check_aspell")
    spell_check_setup();
}

void IckleGUI::exit_cb() {
  exit.emit ();
  destroy ();
}
