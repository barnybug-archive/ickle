/* $Id: IckleGUI.cpp,v 1.62 2002-07-21 00:23:37 bugcreator Exp $
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
#include "Icons.h"
#include "AuthRespDialog.h"
#include "ResendDialog.h"

#include <libicq2000/Client.h>

#include "main.h"

#include "sstream_fix.h"

#include "gtkspell.h"

using std::string;
using std::map;
using std::ostringstream;
using std::endl;

using SigC::bind;

using ICQ2000::ContactRef;

IckleGUI::IckleGUI(MessageQueue& mq)
  :  m_message_queue(mq),
     m_status(ICQ2000::STATUS_OFFLINE),
     m_invisible(false),
     m_top_vbox(false),
     m_contact_scroll(),
     m_contact_list(*this, mq),
     m_away_message( this ),
     m_exiting(false)
{
  // -- libICQ2000 callbacks
  icqclient.messageack.connect(slot(this,&IckleGUI::messageack_cb));
  icqclient.contactlist.connect(slot(this,&IckleGUI::contactlist_cb));
  icqclient.self_contact_status_change_signal.connect(slot(this,&IckleGUI::self_status_change_cb));
  icqclient.self_contact_userinfo_change_signal.connect(slot(this,&IckleGUI::self_userinfo_change_cb));
  icqclient.connecting.connect(slot(this,&IckleGUI::connecting_cb));
  icqclient.disconnected.connect(slot(this,&IckleGUI::disconnected_cb));

  // -- MessageQueue callbacks
  m_message_queue.added.connect(slot(this,&IckleGUI::queue_added_cb));
  m_message_queue.removed.connect(slot(this,&IckleGUI::queue_removed_cb));

  // -- other callbacks
  g_icons.icons_changed.connect( slot( this, &IckleGUI::icons_changed_cb ) );

  Gtk::HButtonBox::set_child_size_default(80,25);
  Gtk::HButtonBox::set_layout_default(GTK_BUTTONBOX_SPREAD);

  set_wmclass( "ickle_main", "ickle_main" );
  set_border_width(5);
  realize();
  set_ickle_title();

  m_contact_scroll.set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  m_contact_scroll.add(m_contact_list);

  m_top_vbox.pack_start(m_contact_scroll,true);

  {
    using namespace Gtk::Menu_Helpers;

    MenuList& ml = m_ickle_menu.items();
    ml.push_back( MenuElem("Add Contact", slot(this, &IckleGUI::add_user_cb)) );
    ml.push_back( MenuElem("My User Info", slot(this, &IckleGUI::my_user_info_cb)) );
    ml.push_back( MenuElem("Search for Contacts", slot(this, &IckleGUI::search_user_cb)) );
    mi_search_for_contacts = ml.back();
    mi_search_for_contacts->set_sensitive(false);
    ml.push_back( MenuElem("Set Auto Response", bind<bool>(slot(this, &IckleGUI::set_auto_response_dialog), false)) );
    ml.push_back( MenuElem("Settings", bind<Gtk::Window*>( slot(this, &IckleGUI::settings_cb), NULL ) ) );
    ml.push_back( MenuElem("About", slot(this, &IckleGUI::about_cb)) );
    ml.push_back( MenuElem("Exit", slot(this, &IckleGUI::exit_cb)) );
    
    MenuList& mbl = m_ickle_menubar.items();
    mbl.push_front( m_status_menu );
    m_status_menu.status_changed_status.connect( slot( this, &IckleGUI::status_menu_status_changed_cb ) );
    m_status_menu.status_changed_invisible.connect( slot( this, &IckleGUI::status_menu_invisible_changed_cb ) );
    m_status_menu.status_changed_status_inv.connect( slot( this, &IckleGUI::status_menu_status_inv_changed_cb ) );
    
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

void IckleGUI::set_ickle_title()
{
  if (m_exiting) return;

  ostringstream ostr;
  ostr << "ickle";

  ContactRef c = icqclient.getSelfContact();
  if (c->getUIN() != 0) {
    ostr << " - " << c->getNameAlias();
  }

  if (m_message_queue.get_size() > 0) ostr << "*";
  set_title(ostr.str());

  if (g_settings.getValueBool("window_status_icons")) {
    Gtk::ImageLoader *p;
    
    if (m_message_queue.get_size() > 0) {
      ostr << "*";
      MessageEvent *ev = m_message_queue.get_first_message();
      if (ev->getServiceType() == MessageEvent::ICQ) {
        ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
        p = g_icons.IconForEvent(icq->getICQMessageType());
      }
    }
    else {
      p = g_icons.IconForStatus( c->getStatus(), c->isInvisible() );
    }
    
    gdk_window_set_icon(get_window(), NULL, p->pix(), p->bit());
  }

}

void IckleGUI::icons_changed_cb()
{
  set_ickle_title();
}

void IckleGUI::queue_added_cb(MessageEvent *ev) {
  if (ev->getServiceType() != MessageEvent::ICQ) return;
  ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
    
  ContactRef c = icq->getICQContact();

  if (m_message_boxes.count(c->getUIN()) == 0) {
    if ( g_settings.getValueBool("message_autopopup") )
      user_popup.emit( c->getUIN() ); // popup a new messagebox
  } else {
    if ( g_settings.getValueBool("message_autoraise") )
      user_popup.emit( c->getUIN() );  // raise existing messagebox
  }

  // update ickle title/icon
  set_ickle_title();
}

void IckleGUI::queue_removed_cb(MessageEvent *) {
  // update ickle title/icon
  set_ickle_title();
}

void IckleGUI::messageack_cb(ICQ2000::MessageEvent *ev) {
  ContactRef c = ev->getContact();

  if (m_message_boxes.count(c->getUIN()) > 0)
    m_message_boxes[c->getUIN()]->messageack_cb(ev);

  if (ev->isFinished() && !ev->isDelivered()) {
    // popup dialog, informing about non-delivery for messages to
    // Occupied/DND (for other failure reasons, it is enough to see
    // the description on the message box status I believe)

    ICQ2000::ICQMessageEvent *icq = dynamic_cast<ICQ2000::ICQMessageEvent*>(ev);
    if (icq != NULL &&
	(ev->getDeliveryFailureReason() == ICQ2000::MessageEvent::Failed_Occupied
	 || ev->getDeliveryFailureReason() == ICQ2000::MessageEvent::Failed_DND)) {
      ResendDialog *p = new ResendDialog(this, icq);
      manage(p);
    }
  }
}

void IckleGUI::contactlist_cb(ICQ2000::ContactListEvent *ev) {
  ContactRef c = ev->getContact();
  unsigned int uin = c->getUIN();
  ICQ2000::ContactListEvent::EventType et = ev->getType();

  if (et == ICQ2000::ContactListEvent::UserRemoved) {
    if (m_message_boxes.count(uin) != 0) {
      MessageBox *m = m_message_boxes[uin];
      m->destroy();
    }

    if (m_userinfo_dialogs.count(uin) != 0) {
      UserInfoDialog *d = m_userinfo_dialogs[uin];
      d->destroy();
    }
  }

}

ContactListView* IckleGUI::getContactListView() {
  return &m_contact_list;
}

void IckleGUI::popup_next_event(const ContactRef& c, History *h) {
  MessageEvent *ev = m_message_queue.get_contact_first_message(c);
  if (ev->getServiceType() == MessageEvent::ICQ) {
    ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
    switch (icq->getICQMessageType()) {
    case ICQMessageEvent::Normal:
    case ICQMessageEvent::URL:
    case ICQMessageEvent::SMS:
    case ICQMessageEvent::SMS_Receipt:
    case ICQMessageEvent::EmailEx:
    case ICQMessageEvent::WebPager:
      popup_messagebox(c, h);
      break;
      
    case ICQMessageEvent::AuthReq:
      popup_auth_req(c, static_cast<AuthReqICQMessageEvent*>(icq));
      break;

    case ICQMessageEvent::AuthAck:
      popup_auth_resp(c, static_cast<AuthAckICQMessageEvent*>(icq));
      break;

    case ICQMessageEvent::UserAdd:
      popup_user_added_you(c, static_cast<UserAddICQMessageEvent*>(icq));
      break;
    }
  }
}

gint IckleGUI::remove_from_queue_idle_cb(MessageEvent *ev)
{
  m_message_queue.remove_from_queue(ev);
  return 0;
}

void IckleGUI::remove_from_queue_delayed(MessageEvent *ev)
{
  // hmmff.. if it's any consolation I don't like this either :-(
  Gtk::Main::idle.connect( bind( slot( this, &IckleGUI::remove_from_queue_idle_cb ), ev ) );
}

void IckleGUI::popup_user_added_you(const ContactRef& c, UserAddICQMessageEvent *ev)
{
  ostringstream ostr;
  ostr << c->getNameAlias() << " has added you to their contact list." << endl;

  PromptDialog *dialog = new PromptDialog( this, PromptDialog::PROMPT_INFO, ostr.str(), false );
  manage( dialog );
  remove_from_queue_delayed(ev);
}

void IckleGUI::popup_auth_req(const ContactRef& c, AuthReqICQMessageEvent *ev)
{
  AuthRespDialog *dialog = new AuthRespDialog(this, c, ev);
  manage( dialog );
  remove_from_queue_delayed(ev);
}

void IckleGUI::popup_auth_resp(const ContactRef& c, AuthAckICQMessageEvent *ev)
{
  ostringstream ostr;
  ostr << c->getNameAlias() << " has " << (ev->isGranted() ? "granted" : "refused")
       << " your request for authorisation." << endl;
  if (!ev->isGranted() && !ev->getMessage().empty()) {
    ostr << "Their refusal message was:" << endl << endl
	 << ev->getMessage() << endl;
  }

  PromptDialog *dialog = new PromptDialog( this, PromptDialog::PROMPT_INFO, ostr.str(), false );
  manage( dialog );
  remove_from_queue_delayed(ev);
}

void IckleGUI::popup_messagebox(const ContactRef& c, History *h)
{
  if (m_message_boxes.count(c->getUIN()) == 0) {
    create_messagebox(c, h);
  } else {
    raise_messagebox(c);
  }
}

void IckleGUI::create_messagebox(const ContactRef& c, History *h)
{
  ContactRef self = icqclient.getSelfContact();
  MessageBox *m = new MessageBox(m_message_queue, self, c, h);
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
  
  if (m_status == ICQ2000::STATUS_OFFLINE) m->offline();
  else m->online();
  
  if (m_userinfo_dialogs.count(c->getUIN()) > 0) m->userinfo_dialog_cb(true);
  
  if (gtkspell_running())
    m->spell_attach();
  
  m->setDisplayTimes(m_display_times);
  m->popup();
}

void IckleGUI::raise_messagebox(const ContactRef& c)
{
  // raise MessageBox
  MessageBox *m = m_message_boxes[c->getUIN()];
  m->raise();
}

void IckleGUI::setGeometry(int x, int y, int w, int h)
{
  geometry_x = x;
  geometry_y = y;
  geometry_w = w;
  geometry_h = h;
  set_default_size(w, h);
  set_uposition(geometry_x, geometry_y);
}

void IckleGUI::show_impl() {
  set_uposition(geometry_x, geometry_y);
  set_default_size(geometry_w, geometry_h);
  Window::show_impl();
}

void IckleGUI::hide_impl() {
  get_window().get_root_origin(geometry_x, geometry_y);
  get_window().get_size(geometry_w, geometry_h);
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

void IckleGUI::status_menu_status_changed_cb(ICQ2000::Status st) {
  
  if (st != ICQ2000::STATUS_ONLINE && st != ICQ2000::STATUS_OFFLINE &&
    g_settings.getValueBool("set_away_response_dialog")) {
    set_auto_response_dialog (true);
  }

  // this forces a reconnect if setting status whilst already connecting, this is the usual
  // behaviour the user wants, as TCP connections can hang, etc.. on dodgy links
  if (st != ICQ2000::STATUS_OFFLINE && icqclient.getStatus() == ICQ2000::STATUS_OFFLINE)
    icqclient.setStatus(ICQ2000::STATUS_OFFLINE);
  
  icqclient.setStatus(st);
}

void IckleGUI::status_menu_status_inv_changed_cb(ICQ2000::Status st, bool inv) {
  
  if (st != ICQ2000::STATUS_ONLINE && st != ICQ2000::STATUS_OFFLINE &&
    g_settings.getValueBool("set_away_response_dialog")) {
    set_auto_response_dialog (true);
  }

  // this forces a reconnect if setting status whilst already connecting, this is the usual
  // behaviour the user wants, as TCP connections can hang, etc.. on dodgy links
  if (st != ICQ2000::STATUS_OFFLINE && icqclient.getStatus() == ICQ2000::STATUS_OFFLINE)
    icqclient.setStatus(ICQ2000::STATUS_OFFLINE);
  
  icqclient.setStatus(st, inv);
}

void IckleGUI::set_auto_response_dialog (bool timeout)
{
  SetAutoResponseDialog *d = new SetAutoResponseDialog(this, auto_response, timeout);
  manage(d);
  d->save_new_msg.connect(slot(this, &IckleGUI::setAutoResponse));
  d->settings_dialog.connect(bind<Gtk::Window*>( slot(this, &IckleGUI::settings_cb), d ));
}

void IckleGUI::status_menu_invisible_changed_cb(bool inv)
{
  icqclient.setInvisible(inv);
}

void IckleGUI::self_status_change_cb(ICQ2000::StatusChangeEvent *ev)
{
  ICQ2000::Status st = ev->getContact()->getStatus();
  bool inv = ev->getContact()->isInvisible();
  
  m_status_menu.status_changed_cb( st, inv );
  m_status = st;
  m_invisible = inv;
  
  map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
  while (i != m_message_boxes.end()) {
    if (st == ICQ2000::STATUS_OFFLINE) (*i).second->offline();
    else (*i).second->online();
    ++i;
  }
  
  // update connection dependent menu entry
  mi_search_for_contacts->set_sensitive(st != ICQ2000::STATUS_OFFLINE);

  // update window title & icon
  set_ickle_title();
}

void IckleGUI::self_userinfo_change_cb(ICQ2000::UserInfoChangeEvent *)
{
  // update window title & icon
  set_ickle_title();
}

void IckleGUI::message_box_close_cb(ContactRef c) {
  unsigned int uin = c->getUIN();
  if (m_message_boxes.count(uin) != 0) m_message_boxes.erase(uin);
  if (m_userinfo_dialogs.count(uin) != 0) m_userinfo_dialogs[uin]->destroy();
}

void IckleGUI::userinfo_dialog_close_cb(ContactRef c) {
  unsigned int uin = c->getUIN();
  if (m_userinfo_dialogs.count(uin) != 0)
    m_userinfo_dialogs.erase(uin);

  if (m_message_boxes.count(uin) != 0)
    m_message_boxes[uin]->userinfo_dialog_cb(false);
}

void IckleGUI::userinfo_dialog_upload_cb(ContactRef c) {
  unsigned int uin = c->getUIN();
  if (m_userinfo_dialogs.count(uin) != 0)
    icqclient.uploadSelfDetails();
}

void IckleGUI::search_user_cb() 
{
  SearchDialog *sd = new SearchDialog(this);
  manage( sd );
}

void IckleGUI::my_user_info_cb()
{
  ContactRef self = icqclient.getSelfContact();
  unsigned int uin = self->getUIN();
  
  if ( m_userinfo_dialogs.count(uin) == 0 ) {
    UserInfoDialog *d = new UserInfoDialog(this, self, true);
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

void IckleGUI::userinfo_fetch_cb(ContactRef c)
{
  icqclient.fetchDetailContactInfo(c);
}

void IckleGUI::my_userinfo_fetch_cb()
{
  icqclient.fetchSelfDetailContactInfo();
}

void IckleGUI::about_cb()
{
  AboutDialog about (this);
  about.run();
}

void IckleGUI::add_user_cb() {
  AddUserDialog *dialog = new AddUserDialog(this);
  manage( dialog );
}

void IckleGUI::invalid_login_prompt() {
  PromptDialog pd(this, PromptDialog::PROMPT_WARNING, "You have not entered a valid UIN and Password. "
	                                     "Go to Settings and correct the details.");
  pd.run();
}

void IckleGUI::turboing_prompt(){
  PromptDialog pd(this, PromptDialog::PROMPT_WARNING, "The server says you have been turboing. "
		  "This means you've been connecting and disconnecting to the server too "
		  "quickly and so it has blocked you temporarily. Don't be alarmed, just "
		  "wait for at least 5 minutes before reattempting. If you still get this "
		  "message then wait a bit longer - maybe up to an hour.");
  pd.run();
}

void IckleGUI::duallogin_prompt() {
  PromptDialog pd(this, PromptDialog::PROMPT_WARNING,
                  "The server recieved multiple simultaneous login for this account.\n"
                  "Do you have multiple clients running with the same account?" );
  pd.run();
}

void IckleGUI::already_running_prompt(const std::string& pid_file, unsigned int pid)
{
  ostringstream ostr;
  ostr << "ickle appears to be already running (process id " << pid << "). " << endl
       << "Either kill this process (if it is actually ickle), or remove the lockfile:" << endl << pid_file;
  PromptDialog pd(this, PromptDialog::PROMPT_WARNING, ostr.str() );
  pd.run();
}

void IckleGUI::disconnect_lowlevel_prompt(int retries) {
  ostringstream os;
  os << "There occured a networking error while communicating with the server, and as a "
    "result you were disconnected.";
  if( retries ) {
    os << " Ickle tried to reconnect " << retries << " times, unfortunately with little success. "
       << "You will have to manually attempt to reconnect, preferably after waiting a short while.";
  }
  
  PromptDialog pd( this, PromptDialog::PROMPT_WARNING, os.str() );
  pd.run();
}

void IckleGUI::disconnect_unknown_prompt(int retries) {
  ostringstream os;
  os << "There occured an unknown error while communicating with the server, and as a "
    "result you were disconnected.";
  if( retries ) {
    os << " Ickle tried to reconnect " << retries << " times, unfortunately with little success. "
       << "You will have to manually attempt to reconnect, preferably after waiting a short while.";
  }
  
  PromptDialog pd( this, PromptDialog::PROMPT_WARNING, os.str() );
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

void IckleGUI::userinfo_toggle_cb(bool b, ContactRef c) {
  unsigned int uin = c->getUIN();
  if ( b && m_userinfo_dialogs.count(uin) == 0 ) {
    popup_userinfo(c);
  } else if ( !b && m_userinfo_dialogs.count(uin) > 0 ) {
    m_userinfo_dialogs[uin]->destroy();
  }
}

void IckleGUI::popup_userinfo(const ContactRef& c) {
  unsigned int uin = c->getUIN();
  if ( m_userinfo_dialogs.count(uin) == 0 ) {
    UserInfoDialog *d = new UserInfoDialog(this, c);
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

void IckleGUI::settings_cb(Gtk::Window * away_dlg)
{
  SettingsDialog dialog (away_dlg ? away_dlg : (Gtk::Window*)this);
  if (away_dlg)
    dialog.raise_away_status_tab();

  if (dialog.run()) {
    bool reconnect = false;
    if (dialog.getUIN() != icqclient.getUIN() ||
	dialog.getPassword() != icqclient.getPassword()) reconnect = icqclient.isConnected();

    if (reconnect) icqclient.setStatus(ICQ2000::STATUS_OFFLINE);
    
    dialog.updateSettings();

    if (dialog.getUIN() != icqclient.getUIN()) icqclient.setUIN(dialog.getUIN());
    if (dialog.getPassword() != icqclient.getPassword()) icqclient.setPassword(dialog.getPassword());
  
    if (reconnect) status_menu_status_changed_cb(ICQ2000::STATUS_ONLINE);

    settings_changed.emit();
  }

}

void IckleGUI::spell_check_setup()
{
  if (g_settings.getValueBool("spell_check")) {
    // start gtkspell

    char spell_language[128];
	
    char *ispellcmd[] = { "ispell", "-a", "-d","place holder",NULL };
    char *aspellcmd[] = { "aspell", "pipe", "place holder", NULL };
    char **spellcmd;

    if (g_settings.getValueString("spell_check_lang").size()>0) {
    //some language was specified
    	strncpy (spell_language, g_settings.getValueString("spell_check_lang").c_str(), 100);
	char language_parm[128];
        strncpy (language_parm, "--lang=",8);
        strncat (language_parm,spell_language,100);
        aspellcmd[2] = language_parm;
        ispellcmd[3] = spell_language;
    } else {// no language? then system default
	aspellcmd[2] = NULL;
	ispellcmd[2] = NULL;
    }
    
	if (g_settings.getValueBool("spell_check_aspell"))
	    spellcmd = aspellcmd;    
	else
            spellcmd = ispellcmd;
          
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
  if ((k == "spell_check") || (k == "spell_check_lang") || (k == "spell_check_aspell"))
    spell_check_setup();
}

void IckleGUI::exit_cb() {
  m_exiting = true;
  exit.emit ();
  destroy ();
}

void IckleGUI::connecting_cb(ICQ2000::ConnectingEvent *) {
  m_status_menu.connecting();
}

void IckleGUI::disconnected_cb(ICQ2000::DisconnectedEvent *)
{
  // ensure StatusMenu is set back to offline, ie. for when connecting fails
  m_status_menu.set_status(ICQ2000::STATUS_OFFLINE, icqclient.getInvisibleWanted());
}
