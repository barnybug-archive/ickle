/*
 * AuthRespDialog
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#include "AuthRespDialog.h"

#include <gtk--/box.h>
#include <gtk--/radiobutton.h>

#include <libicq2000/Client.h>
#include "main.h"
#include "sstream_fix.h"

using std::ostringstream;

AuthRespDialog::AuthRespDialog(const ICQ2000::ContactRef& contact, AuthReqICQMessageEvent *ev)
  : Gtk::Dialog(),
    m_ok("Send Response"), m_cancel("Ignore"),
    m_grant("Grant", 0), m_refuse("Refuse", 0),
    m_contact(contact), m_label("Enter your refusal message:", 0)
{
  set_title("Authorisation Response");

  m_ok.clicked.connect(slot(this,&AuthRespDialog::ok_cb));
  m_cancel.clicked.connect( destroy.slot() );
  
  Gtk::VBox *vbox = get_vbox();
  
  ostringstream ostr;
  ostr << contact->getNameAlias() << " is requesting authorisation." << endl
       << "You should grant or refuse the request." << endl
       << "Their request message is: " << endl
       << ev->getMessage() << endl;
  
  Gtk::Label *label = manage( new Gtk::Label( ostr.str(), 0 ) );
  label->set_justify(GTK_JUSTIFY_FILL);
  label->set_line_wrap(true);
  vbox->pack_start( *label );

  m_grant.clicked.connect( slot( this, &AuthRespDialog::grant_clicked_cb ) );
  vbox->pack_start( m_grant );

  m_refuse.clicked.connect( slot( this, &AuthRespDialog::refuse_clicked_cb ) );
  m_refuse.set_group( m_grant.group() );
  vbox->pack_start( m_refuse );
  m_grant.set_active(true);
  m_text.set_editable(true);

  // by default grant is selected - no message for grant
  m_text.set_sensitive(false);
  m_label.set_sensitive(false);
  vbox->pack_start( m_label );


  vbox->pack_start( m_text );

  Gtk::HBox *hbox = get_action_area();
  hbox->pack_start(m_ok, true, true, 0);
  hbox->pack_start(m_cancel, true, true, 0);

  set_default_size(250,100);
  set_border_width(10);
  show_all();
}

void AuthRespDialog::grant_clicked_cb()
{
  m_text.set_sensitive(false);
  m_label.set_sensitive(false);
}

void AuthRespDialog::refuse_clicked_cb()
{
  m_text.set_sensitive(true);
  m_label.set_sensitive(true);
}

void AuthRespDialog::ok_cb() {
  ICQ2000::AuthAckEvent *ev = new ICQ2000::AuthAckEvent(m_contact, m_text.get_chars(0,-1), m_grant.get_active());
  icqclient.SendEvent(ev);
  destroy.emit();
}
