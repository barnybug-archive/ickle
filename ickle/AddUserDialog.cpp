/*
 * AddUserDialog
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

#include "AddUserDialog.h"

#include "main.h"
#include <libicq2000/Client.h>

#include <gtk--/table.h>
#include <gtk--/buttonbox.h>

AddUserDialog::AddUserDialog(Gtk::Window * parent)
  : Gtk::Dialog(),
    m_ok("OK"), m_cancel("Cancel"),
    m_icq_user("An ICQ contact (has a uin)", 0),
    m_alert_check("Alert User", 0),
    m_mobile_user("A Mobile contact (cellular)", 0),
    m_mode_frame("Type of contact"),
    m_icq_frame("ICQ Contacts"),
    m_mobile_frame("Mobile Contacts"),
    m_alias_label("Alias", 0),
    m_mobileno_label("Mobile Number", 0, 1),
    m_uin_label("User Number (UIN)", 0)
{
  Gtk::Label *label;

  set_title("Add Contact");
  set_transient_for (*parent);

  m_ok.clicked.connect(slot(this,&AddUserDialog::ok_cb));
  m_cancel.clicked.connect( destroy.slot() );

  Gtk::HBox *hbox = get_action_area();
  hbox->set_border_width(0);
  Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  hbbox->pack_start(m_ok, true, true, 0);
  hbbox->pack_start(m_cancel, true, true, 0);
  hbox->pack_start( *hbbox );
  
  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  Gtk::VBox *vbox2;
  Gtk::Table *table;

  // -- mode selection frame

  vbox2 = manage( new Gtk::VBox() );
  m_icq_user.clicked.connect( slot( this, &AddUserDialog::update_stuff ) );
  m_mobile_user.clicked.connect( slot( this, &AddUserDialog::update_stuff ) );
  m_mobile_user.set_group( m_icq_user.group() );
  m_icq_user.set_active(true);
  vbox2->set_border_width(5);
  vbox2->pack_start( m_icq_user );
  vbox2->pack_start( m_mobile_user );

  m_mode_frame.set_border_width(0);
  m_mode_frame.add( *vbox2 );
  vbox->pack_start( m_mode_frame );

  // -- icq frame
  table = manage( new Gtk::Table( 2, 2, false ) );
  
  table->attach( m_uin_label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 5 );
  m_uin_entry.changed.connect( slot( this, &AddUserDialog::uin_changed_cb ) );
  m_uin_entry.set_max_length(12);
  table->set_border_width(5);
  table->attach( m_uin_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND );
  table->attach( m_alert_check, 0, 2, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );
  m_icq_frame.set_border_width(0);
  m_icq_frame.add( *table );
  vbox->pack_start( m_icq_frame );

  // -- mobile frame
  table = manage( new Gtk::Table( 2, 2, false ) );

  table->attach( m_alias_label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 5);
  table->attach( m_alias_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

  table->attach( m_mobileno_label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 5);
  m_mobileno_entry.changed.connect( slot( this, &AddUserDialog::mobileno_changed_cb ) );
  table->attach( m_mobileno_entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

  m_mobile_frame.set_border_width(0);
  m_mobile_frame.add( *table );
  table->set_border_width(5);
  vbox->pack_start( m_mobile_frame );

  set_border_width(10);
  show_all();
}

void AddUserDialog::ok_cb() {

  if (m_icq_user.get_active()) {
    unsigned int uin = ICQ2000::Contact::StringtoUIN(m_uin_entry.get_text());
    if (uin == 0) return;
    
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

  } else {

    if (!m_mobileno_entry.get_text().empty()) {
      ICQ2000::ContactRef nc(new ICQ2000::Contact(m_alias_entry.get_text()));
      nc->setMobileNo(m_mobileno_entry.get_text());
      icqclient.addContact( nc );
    }

  }
  
  destroy.emit();
}

void AddUserDialog::update_stuff()
{
  if (m_icq_user.get_active()) {
    
    // set uin stuff sensitive
    m_uin_label.set_sensitive(true);
    m_uin_entry.set_sensitive(true);
    m_alert_check.set_sensitive(true);

    // set mobile stuff insensitive
    m_alias_label.set_sensitive(false);
    m_alias_entry.set_sensitive(false);
    m_mobileno_label.set_sensitive(false);
    m_mobileno_entry.set_sensitive(false);

    unsigned int uin = ICQ2000::Contact::StringtoUIN(m_uin_entry.get_text());
    m_ok.set_sensitive( uin != 0 );
    
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

    m_ok.set_sensitive( !m_mobileno_entry.get_text().empty() );
  }
}

void AddUserDialog::uin_changed_cb()
{
  if (!m_icq_user.get_active()) return;
  unsigned int uin = ICQ2000::Contact::StringtoUIN(m_uin_entry.get_text());
  m_ok.set_sensitive( uin != 0 );
}

void AddUserDialog::mobileno_changed_cb()
{
  if (m_icq_user.get_active()) return;
  m_ok.set_sensitive( !m_mobileno_entry.get_text().empty() );
}
