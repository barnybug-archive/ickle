/* $Id: IckleGUI.cpp,v 1.75 2003/02/10 00:43:26 barnabygray Exp
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
#include "AboutDialog.h"
#include "PromptDialog.h"
#include "Icons.h"
#include "AuthRespDialog.h"
#include "ResendDialog.h"
#include "ReceiveFileDialog.h"

#include "AddContactDialog.h"
#include "SearchDialog.h"

#include "ContactListView.h"

#include <gtkmm/checkmenuitem.h>
#include <gtkmm/stock.h>

#include <libicq2000/Client.h>

#include "pixmaps/info.xpm"

#include "ickle.h"
#include "ucompose.h"
#include "main.h"

// TODO #include "gtkspell.h"

using std::string;
using std::endl;

using ICQ2000::ContactRef;

IckleGUI::IckleGUI(MessageQueue& mq, HistoryMap& histmap)
  :  m_message_queue(mq),
     m_status(ICQ2000::STATUS_OFFLINE),
     m_invisible(false),
     m_top_vbox(false),
     m_contact_scroll(),
     m_contact_list( new ContactListView( *this, mq ) ),
     m_away_message( *this ),
     m_exiting(false),
     m_log_window(),
     m_histmap(histmap)
{
  // -- libICQ2000 callbacks
  icqclient.messageack.connect(this,&IckleGUI::messageack_cb);
  icqclient.contactlist.connect(this,&IckleGUI::contactlist_cb);
  icqclient.self_contact_status_change_signal.connect(this,&IckleGUI::self_status_change_cb);
  icqclient.self_contact_userinfo_change_signal.connect(this,&IckleGUI::self_userinfo_change_cb);
  icqclient.connecting.connect(this,&IckleGUI::connecting_cb);
  icqclient.disconnected.connect(this,&IckleGUI::disconnected_cb);

  // -- MessageQueue callbacks
  m_message_queue.added.connect(SigC::slot(*this,&IckleGUI::queue_added_cb));
  m_message_queue.removed.connect(SigC::slot(*this,&IckleGUI::queue_removed_cb));

  // -- other callbacks
  g_icons.icons_changed.connect( SigC::slot( *this, &IckleGUI::icons_changed_cb ) );

  // TODO - how to set HButtonBox defaults ?

  set_wmclass( "ickle_main", "ickle_main" );
  set_border_width(5);
  realize();
  set_ickle_title();

  m_contact_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  m_contact_scroll.add(* Gtk::manage( m_contact_list ) );

  m_top_vbox.pack_start(m_contact_scroll);

  {
    using namespace Gtk::Menu_Helpers;

    MenuList& ml = m_ickle_menu.items();
    ml.push_back( ImageMenuElem( _("Add Contact"),
				* manage( new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_MENU) ),
				SigC::slot(*this, &IckleGUI::add_contact_cb)) );
    ml.push_back( ImageMenuElem( _("My User Info"),
				 * manage( new Gtk::Image( Gdk::Pixbuf::create_from_xpm_data( info_xpm ) ) ),
				 SigC::slot(*this, &IckleGUI::my_user_info_cb)) );
    ml.push_back( ImageMenuElem( _("Search for Contacts"),
				* manage( new Gtk::Image(Gtk::Stock::FIND, Gtk::ICON_SIZE_MENU) ),
				SigC::slot(*this, &IckleGUI::search_contact_cb)) );
    mi_search_for_contacts = &ml.back();
    mi_search_for_contacts->set_sensitive(false);
    ml.push_back( ImageMenuElem( _("Set Auto Response"),
				 * manage( new Gtk::Image( g_icons.get_icon_for_status( ICQ2000::STATUS_AWAY, false ) ) ),
				 SigC::bind<bool>(SigC::slot(*this, &IckleGUI::set_auto_response_dialog), false)) );

    m_offline_co_mi = manage( new Gtk::CheckMenuItem( _("Show offline contacts") ) );
    m_offline_co_mi->signal_toggled().connect( SigC::slot(*this, &IckleGUI::toggle_offline_co_cb) );
    m_ickle_menu.append(*m_offline_co_mi);

    m_log_window_mi = manage( new Gtk::CheckMenuItem( _("Log Window") ) );
    m_log_window_mi->signal_toggled().connect( SigC::slot(*this, &IckleGUI::log_window_cb) );
    m_ickle_menu.append(*m_log_window_mi);

    ml.push_back( StockMenuElem( Gtk::Stock::PREFERENCES, SigC::slot(*this, &IckleGUI::settings_cb) ) );
    ml.push_back( ImageMenuElem( _("About"),
				* manage( new Gtk::Image(Gtk::Stock::DIALOG_INFO, Gtk::ICON_SIZE_MENU) ),
				SigC::slot(*this, &IckleGUI::about_cb)) );
    ml.push_back( StockMenuElem( Gtk::Stock::QUIT, SigC::slot(*this, &IckleGUI::exit_cb) ) );
    
    MenuList& mbl = m_ickle_menubar.items();
    m_ickle_menubar.append( m_status_menu );
    //    mbl.push_front( m_status_menu );
    m_status_menu.status_changed_status.connect( SigC::slot( *this, &IckleGUI::status_menu_status_changed_cb ) );
    m_status_menu.status_changed_invisible.connect( SigC::slot( *this, &IckleGUI::status_menu_invisible_changed_cb ) );
    m_status_menu.status_changed_status_inv.connect( SigC::slot( *this, &IckleGUI::status_menu_status_inv_changed_cb ) );

    mbl.front().set_right_justified(true);
    mbl.push_front(MenuElem( _("ickle"), m_ickle_menu));
  }
  
  m_top_vbox.pack_end(m_ickle_menubar, Gtk::PACK_SHRINK);
  
  add(m_top_vbox);

  m_log_window.signal_hide().connect( SigC::slot( *this, &IckleGUI::log_window_hidden_cb ) );

  m_contact_list->signal_messagebox_popup().connect( SigC::slot( *this, &IckleGUI::messagebox_popup_cb ) );
  m_contact_list->signal_userinfo_popup().connect( SigC::slot( *this, &IckleGUI::popup_userinfo ) );
  m_contact_list->grab_focus();

  g_settings.settings_changed.connect( SigC::slot( *this, &IckleGUI::settings_changed_cb ) );
}

IckleGUI::~IckleGUI()
{
  while(!m_message_boxes.empty())
  {
    std::map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
    delete i->second;
  }

  while(!m_userinfo_dialogs.empty())
  {
    std::map<unsigned int, UserInfoDialog*>::iterator i = m_userinfo_dialogs.begin();
    delete i->second;
  }

  // TODO gtkspell
  //  if (gtkspell_running())
  //gtkspell_stop();
  m_signal_destroy.emit();
}

void IckleGUI::set_ickle_title()
{
  if (m_exiting) return;

  ICQ2000::ContactRef c = icqclient.getSelfContact();
  if (c->getUIN() == 0)
  {
    set_title( _("ickle") );
  }
  else
  {
    set_title( String::ucompose( _("ickle - %1%2"),
				 Glib::ustring(c->getNameAlias()),
				 ( m_message_queue.get_size() > 0 ? "*" : "" ) ) );
  }

  if (g_settings.getValueBool("window_status_icons"))
  {
    Glib::RefPtr<Gdk::Pixbuf> p;

    if (m_message_queue.get_size() > 0)
    {
      MessageEvent *ev = m_message_queue.get_first_message();
      if (ev->getServiceType() == MessageEvent::ICQ) {
        ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
        p = g_icons.get_icon_for_event(icq->getICQMessageType());
      }
    }
    else
    {
      p = g_icons.get_icon_for_status( c->getStatus(), c->isInvisible() );
    }

    // TODO - set icon list ?
    set_icon(p);
  }

}


void IckleGUI::change_client()
{
  signal_restart_client.emit();
}


void IckleGUI::icons_changed_cb()
{
  set_ickle_title();
}

void IckleGUI::queue_added_cb(MessageEvent *ev)
{
  if (ev->getServiceType() != MessageEvent::ICQ) return;
  ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);

  ContactRef c = icq->getICQContact();

  if (m_message_boxes.count(c->getUIN()) == 0)
  {
    if ( g_settings.getValueBool("message_autopopup") )
      messagebox_popup_cb(c->getUIN()); // popup a new messagebox
  }
  else
  {
    if ( g_settings.getValueBool("message_autoraise") )
      messagebox_popup_cb(c->getUIN()); // raise existing messagebox
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
      new ResendDialog(*this, icq);
    }
  }
}

void IckleGUI::contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  ICQ2000::ContactListEvent::EventType et = ev->getType();

  if (et == ICQ2000::ContactListEvent::UserRemoved) {
    ICQ2000::UserRemovedEvent *cev = static_cast<ICQ2000::UserRemovedEvent*>(ev);
    ContactRef c = cev->getContact();
    unsigned int uin = c->getUIN();

    if (m_message_boxes.count(uin) != 0)
    {
      MessageBox *m = m_message_boxes[uin];
      delete m;
    }

    if (m_userinfo_dialogs.count(uin) != 0)
    {
      UserInfoDialog *d = m_userinfo_dialogs[uin];
      delete d;
    }
  }

}

ContactListView* IckleGUI::getContactListView() {
  return m_contact_list;
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
    case ICQMessageEvent::FileTransfer:
      popup_file_transfer_request(c, static_cast<FileTransferICQMessageEvent*>(icq));
      break;
    }
  }
}

bool IckleGUI::remove_from_queue_idle_cb(MessageEvent *ev)
{
  m_message_queue.remove_from_queue(ev);
  return false;
}

void IckleGUI::remove_from_queue_delayed(MessageEvent *ev)
{
  // hmmff.. if it's any consolation I don't like this either :-(
  Glib::signal_idle().connect( SigC::bind( SigC::slot( *this, &IckleGUI::remove_from_queue_idle_cb ), ev ) );
}

void IckleGUI::popup_user_added_you(const ContactRef& c, UserAddICQMessageEvent *ev)
{
  Glib::ustring str = String::ucompose( _("%1 has added you to their contact list."),
				 Glib::ustring(c->getNameAlias()) );

  new PromptDialog( *this, Gtk::MESSAGE_INFO, str, false );
  remove_from_queue_delayed(ev);
}

void IckleGUI::popup_file_transfer_request(const ContactRef& c, FileTransferICQMessageEvent *ev)
{
  ReceiveFileDialog *dialog = new ReceiveFileDialog( this, c, ev );
  remove_from_queue_delayed(ev);
}

void IckleGUI::popup_auth_req(const ContactRef& c, AuthReqICQMessageEvent *ev)
{
  new AuthRespDialog(*this, c, ev);
  remove_from_queue_delayed(ev);
}

void IckleGUI::popup_auth_resp(const ContactRef& c, AuthAckICQMessageEvent *ev)
{
  Glib::ustring str;
  if (ev->isGranted())
    str = String::ucompose( _("%1 has granted your request for authorisation."), Glib::ustring(c->getNameAlias()) );
  else
    str = String::ucompose( _("%1 has refused your request for authorisation."), Glib::ustring(c->getNameAlias()) );

  str += "\n";
  
  if (!ev->isGranted() && !ev->getMessage().empty())
  {
    str += String::ucompose( _("Their refusal message was:\n\n%1\n"), Glib::ustring(ev->getMessage()) );
  }

  new PromptDialog( *this, Gtk::MESSAGE_INFO, str, false );
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

  m->signal_destroy().connect(SigC::bind(SigC::slot(*this,&IckleGUI::messagebox_destroy_cb), c));
  m->signal_send_event().connect(m_signal_send_event.slot());
  m->signal_userinfo_dialog().connect(SigC::bind(SigC::slot(*this,&IckleGUI::userinfo_toggle_cb), c));
  m_message_boxes[c->getUIN()] = m;
  
  if (m_status == ICQ2000::STATUS_OFFLINE)
    m->offline();
  else
    m->online();
  
  if (m_userinfo_dialogs.count(c->getUIN()) > 0) m->userinfo_dialog_cb(true);
  
  // TODO gtkspell
  //  if (gtkspell_running())
  //    m->spell_attach();
  
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
  move(geometry_x, geometry_y);
}

void IckleGUI::on_show()
{
  move(geometry_x, geometry_y);
  set_default_size(geometry_w, geometry_h);
  Gtk::Window::on_show();
}

void IckleGUI::on_hide()
{
  get_window()->get_root_origin(geometry_x, geometry_y);
  get_window()->get_size(geometry_w, geometry_h);
  Gtk::Window::on_hide();
}

void IckleGUI::status_menu_status_changed_cb(ICQ2000::Status st)
{
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
  SetAutoResponseDialog *d = new SetAutoResponseDialog(*this, g_settings.getValueString("last_auto_response"), timeout);
  d->save_new_msg.connect(SigC::slot(*this, &IckleGUI::setAutoResponse));
  d->settings_dialog.connect(SigC::bind<Gtk::Window*>( SigC::slot(*this, &IckleGUI::settings_away_cb), d ));
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

  std::map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
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

void IckleGUI::messagebox_destroy_cb(ContactRef c)
{
  unsigned int uin = c->getUIN();

  if (m_message_boxes.count(uin) != 0)
    m_message_boxes.erase(uin);

  if (m_userinfo_dialogs.count(uin) != 0)
    delete m_userinfo_dialogs[uin];
}

void IckleGUI::userinfo_dialog_destroy_cb(ContactRef c)
{
  unsigned int uin = c->getUIN();
  if (m_userinfo_dialogs.count(uin) != 0)
    m_userinfo_dialogs.erase(uin);

  if (m_message_boxes.count(uin) != 0)
    m_message_boxes[uin]->userinfo_dialog_cb(false);
}

void IckleGUI::userinfo_dialog_upload_cb(ContactRef c)
{
  unsigned int uin = c->getUIN();
  if (m_userinfo_dialogs.count(uin) != 0)
    icqclient.uploadSelfDetails();
}

void IckleGUI::search_contact_cb() 
{
  new SearchDialog(this);
}

void IckleGUI::my_user_info_cb()
{
  ContactRef self = icqclient.getSelfContact();
  unsigned int uin = self->getUIN();
  
  if ( m_userinfo_dialogs.count(uin) == 0 ) {
    UserInfoDialog *d = new UserInfoDialog(*this, self, true);
    d->signal_closed().connect(SigC::bind(SigC::slot(*this,&IckleGUI::userinfo_dialog_destroy_cb), self));
    d->signal_fetch().connect( SigC::slot( *this, &IckleGUI::my_userinfo_fetch_cb ) );
    d->signal_upload().connect( SigC::bind( SigC::slot( *this, &IckleGUI::userinfo_dialog_upload_cb ), self));
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
  AboutDialog about(*this);
  about.run();
}

void IckleGUI::add_contact_cb()
{
  new AddContactDialog(*this);
}

void IckleGUI::toggle_offline_co_cb()
{
  m_contact_list->set_show_offline_contacts( m_offline_co_mi->get_active() );
  g_settings.setValue("show_offline_contacts", m_offline_co_mi->get_active() );
}

void IckleGUI::log_window_cb()
{
  if ( m_log_window_mi->get_active() )
    m_log_window.show();
  else
    m_log_window.hide();
}

void IckleGUI::log_window_hidden_cb()
{
  if (is_realized()) m_log_window_mi->set_active(false);
}

void IckleGUI::invalid_login_prompt()
{
  PromptDialog pd(*this, Gtk::MESSAGE_WARNING,
		  _("You have not entered a valid UIN and Password. "
		    "Go to Settings and correct the details.") );
  pd.run();
}

void IckleGUI::turboing_prompt()
{
  PromptDialog pd(*this, Gtk::MESSAGE_WARNING,
		  _("The server says you have been turboing. "
		    "This means you've been connecting and disconnecting to the server too "
		    "quickly and so it has blocked you temporarily. Don't be alarmed, just "
		    "wait for at least 5 minutes before reattempting. If you still get this "
		    "message then wait a bit longer - maybe up to an hour.") );
  pd.run();
}

void IckleGUI::duallogin_prompt()
{
  PromptDialog pd(*this, Gtk::MESSAGE_WARNING,
                  _("The server recieved multiple simultaneous login for this account.\n"
		    "Do you have multiple clients running with the same account?") );
  pd.run();
}

void IckleGUI::already_running_prompt(const std::string& pid_file, unsigned int pid)
{
  PromptDialog pd(*this, Gtk::MESSAGE_WARNING,
		  String::ucompose( _("ickle appears to be already running (process id %1).\n"
				      "Either kill this process (if it is actually ickle), or remove the lockfile:\n%2"), pid, pid_file) );
  pd.run();
}

void IckleGUI::disconnect_lowlevel_prompt(int retries)
{
  Glib::ustring str;

  str = _("There occured a networking error while communicating with the server, and as a "
	  "result you were disconnected.");
  if( retries )
  {
    str += String::ucompose( _(" Ickle tried to reconnect %1 times, unfortunately with little success.\n"
			       "You will have to manually attempt to reconnect, preferably after waiting a short while."), retries );
    // TODO plural form
  }
  
  PromptDialog pd( *this, Gtk::MESSAGE_WARNING, str );
  pd.run();
}

void IckleGUI::disconnect_unknown_prompt(int retries)
{
  Glib::ustring str;
  
  str = _("There occured an unknown error while communicating with the server, and as a "
	  "result you were disconnected.");
  if( retries )
  {
    str += String::ucompose( _(" Ickle tried to reconnect %1 times, unfortunately with little success.\n"
			       "You will have to manually attempt to reconnect, preferably after waiting a short while."),
			     retries );
  }
  
  PromptDialog pd( *this, Gtk::MESSAGE_WARNING, str );
  pd.run();
}

void IckleGUI::setDisplayTimes(bool d)
{
  if (m_display_times != d)
  {
    m_display_times = d;
    
    std::map<unsigned int, MessageBox*>::iterator i = m_message_boxes.begin();
    while (i != m_message_boxes.end())
    {
      (*i).second->setDisplayTimes(d);
      ++i;
    }

  }
}

void IckleGUI::setAutoResponse(const string& ar)
{
  g_settings.setValue("last_auto_response", ar);
}

void IckleGUI::userinfo_toggle_cb(bool b, ContactRef c) {
  unsigned int uin = c->getUIN();
  if ( b && m_userinfo_dialogs.count(uin) == 0 )
  {
    popup_userinfo(c);
  }
  else if ( !b && m_userinfo_dialogs.count(uin) > 0 )
  {
    delete m_userinfo_dialogs[uin];
  }
}

void IckleGUI::popup_userinfo(const ContactRef& c) {
  unsigned int uin = c->getUIN();
  if ( m_userinfo_dialogs.count(uin) == 0 ) {
    UserInfoDialog *d = new UserInfoDialog(*this, c, false);
    d->signal_closed().connect(SigC::bind(SigC::slot(*this,&IckleGUI::userinfo_dialog_destroy_cb), c));
    d->signal_fetch().connect( SigC::bind( SigC::slot(*this, &IckleGUI::userinfo_fetch_cb), c) );
    m_userinfo_dialogs[ uin ] = d;
    if (m_message_boxes.count(uin) != 0) {
      m_message_boxes[uin]->userinfo_dialog_cb(true);
    }
  } else {
    m_userinfo_dialogs[ uin ]->raise();
  }

}

void IckleGUI::show_settings_dialog(Gtk::Window& w, bool away)
{
  SettingsDialog dialog(w, away);
  dialog.change_client.connect(SigC::slot(*this, &IckleGUI::change_client) );
  
  if (dialog.run() == Gtk::RESPONSE_OK)
  {
    m_signal_save_settings.emit();
  }
}

void IckleGUI::settings_cb()
{
  show_settings_dialog(*this, false);
}

void IckleGUI::settings_away_cb(Gtk::Window * w)
{
  show_settings_dialog(*w, true);
}

void IckleGUI::spell_check_setup()
{
  /* TODO gtkspell
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
  */
}


void IckleGUI::settings_changed_cb(const string& k)
{
  if ((k == "spell_check") || (k == "spell_check_lang") || (k == "spell_check_aspell"))
    spell_check_setup();
}

void IckleGUI::exit_cb()
{
  m_exiting = true;
  m_signal_exit.emit();
}

bool IckleGUI::on_delete_event(GdkEventAny *event)
{
  m_exiting = true;
  m_signal_exit.emit();
  return true;
}

void IckleGUI::connecting_cb(ICQ2000::ConnectingEvent *)
{
  m_status_menu.connecting();
}

void IckleGUI::disconnected_cb(ICQ2000::DisconnectedEvent *)
{
  // ensure StatusMenu is set back to offline, ie. for when connecting fails
  m_status_menu.set_status(ICQ2000::STATUS_OFFLINE, icqclient.getInvisibleWanted());
}

void IckleGUI::post_settings_loaded()
{
  bool offline_co = g_settings.getValueBool("show_offline_contacts");
  m_offline_co_mi->set_active( offline_co );
  m_contact_list->set_show_offline_contacts( offline_co );
}

void IckleGUI::messagebox_popup_cb(unsigned int uin)
{
  ICQ2000::ContactRef c = icqclient.getContact(uin);
  if (m_message_queue.get_contact_size(c) == 0)
    popup_messagebox(c, m_histmap[c->getUIN()]);
  else
    popup_next_event(c, m_histmap[c->getUIN()]);
}

SigC::Signal0<void>& IckleGUI::signal_exit()
{
  return m_signal_exit;
}

SigC::Signal0<void>& IckleGUI::signal_destroy()
{
  return m_signal_destroy;
}

SigC::Signal0<void>& IckleGUI::signal_save_settings()
{
  return m_signal_save_settings;
}

SigC::Signal1<void, ICQ2000::MessageEvent*>& IckleGUI::signal_send_event()
{
  return m_signal_send_event;
}
