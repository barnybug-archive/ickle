/*
 * AddContactDialog
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

#include "AddContactDialog.h"

#include "main.h"
#include <libicq2000/Client.h>

#include <gtkmm/table.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>

#include "ickle.h"

AddContactDialog::AddContactDialog(Gtk::Window& parent)
  : Gtk::Dialog( _("Add Contact"), parent),
    m_icq_contact( _("An ICQ contact (has a uin)"), 0),
    m_mobile_contact( _("A Mobile contact (cellular)"), 0),
    m_uin_label( _("User Number (UIN)"), 0),
    m_alert_check( _("Alert Contact"), 0),
    m_mode_frame( _("Type of contact") ),
    m_icq_frame( _("ICQ Contacts") ),
    m_mobile_frame( _("Mobile Contacts") ),
    m_group_frame( _("Group") ),
    m_alias_label( _("Alias"), 0),
    m_mobileno_label( _("Mobile Number"), 0, 1)
{
  set_position(Gtk::WIN_POS_CENTER);

  add_button(Gtk::Stock::ADD, Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  Gtk::VBox *vbox2;
  Gtk::Table *table;

  // -- mode selection frame

  vbox2 = manage( new Gtk::VBox() );
  m_icq_contact.signal_clicked().connect( SigC::slot( *this, &AddContactDialog::update_stuff ) );
  m_mobile_contact.signal_clicked().connect( SigC::slot( *this, &AddContactDialog::update_stuff ) );

  Gtk::RadioButton::Group group = m_icq_contact.get_group();
  m_mobile_contact.set_group( group );
  m_icq_contact.set_active(true);
  vbox2->set_border_width(5);
  vbox2->pack_start( m_icq_contact );
  vbox2->pack_start( m_mobile_contact );

  m_mode_frame.set_border_width(0);
  m_mode_frame.add( *vbox2 );
  vbox->pack_start( m_mode_frame );

  // -- icq frame
  table = manage( new Gtk::Table( 2, 2, false ) );
  
  table->attach( m_uin_label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5 );
  m_uin_entry.signal_changed().connect( SigC::slot( *this, &AddContactDialog::uin_changed_cb ) );
  m_uin_entry.set_max_length(12);
  table->set_border_width(5);
  table->attach( m_uin_entry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND );
  table->attach( m_alert_check, 0, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  m_icq_frame.set_border_width(0);
  m_icq_frame.add( *table );
  vbox->pack_start( m_icq_frame );

  // -- mobile frame
  table = manage( new Gtk::Table( 2, 2, false ) );

  table->attach( m_alias_label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5);
  table->attach( m_alias_entry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

  table->attach( m_mobileno_label, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5);
  m_mobileno_entry.signal_changed().connect( SigC::slot( *this, &AddContactDialog::mobileno_changed_cb ) );
  table->attach( m_mobileno_entry, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

  m_mobile_frame.set_border_width(0);
  m_mobile_frame.add( *table );
  table->set_border_width(5);
  vbox->pack_start( m_mobile_frame );

  set_border_width(10);
  show_all();
}

void AddContactDialog::on_response(int response_id)
{

  if (response_id == Gtk::RESPONSE_OK)
  {
    if (m_icq_contact.get_active())
    {
      unsigned int uin = ICQ2000::Contact::StringtoUIN(m_uin_entry.get_text());
      if (uin == 0) return;
    
      /* TODO
	 ICQ2000::ContactRef c = icqclient.getContact(uin);
	 if (c.get() == NULL) {
	 ICQ2000::ContactRef nc(new ICQ2000::Contact(uin));
	 icqclient.addContact(nc);
	 if (m_alert_check.get_active()) {
	 ICQ2000::UserAddEvent *ev = new ICQ2000::UserAddEvent(nc);
	 icqclient.SendEvent(ev);
	 }
	 // fetch user info
	 icqclient.fetchDetailContactInfo(nc);
	 }
      */

    }
    else
    {

      /* TODO
	 if (!m_mobileno_entry.get_text().empty()) {
	 ICQ2000::ContactRef nc(new ICQ2000::Contact(m_alias_entry.get_text()));
	 nc->setMobileNo(m_mobileno_entry.get_text());
	 icqclient.addContact( nc );
	 }
      */

    }
  }

  delete this;
}

void AddContactDialog::update_stuff()
{
  if (m_icq_contact.get_active()) {
    
    // set uin stuff sensitive
    m_uin_label.set_sensitive(true);
    m_uin_entry.set_sensitive(true);
    m_alert_check.set_sensitive(true);

    // set mobile stuff insensitive
    m_alias_label.set_sensitive(false);
    m_alias_entry.set_sensitive(false);
    m_mobileno_label.set_sensitive(false);
    m_mobileno_entry.set_sensitive(false);

    // unsigned int uin = ICQ2000::Contact::StringtoUIN(m_uin_entry.get_text());
    // TODO: change sensitivity of OK
    
  } else {

    // set mobile stuff sensitive
    m_alias_label.set_sensitive(true);
    m_alias_entry.set_sensitive(true);
    m_mobileno_label.set_sensitive(true);
    m_mobileno_entry.set_sensitive(true);

    // set uin stuff insensitive
    m_uin_label.set_sensitive(false);
    m_uin_entry.set_sensitive(false);
    m_alert_check.set_sensitive(false);

    // TODO: change sensitivity of OK
  }
}

void AddContactDialog::uin_changed_cb()
{
  if (!m_icq_contact.get_active()) return;
  // unsigned int uin = ICQ2000::Contact::StringtoUIN(m_uin_entry.get_text());
  // TODO: change sensitivity of OK
}

void AddContactDialog::mobileno_changed_cb()
{
  if (m_icq_contact.get_active()) return;
  // TODO: change sensitivity of OK
}
