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

#include <gtkmm/box.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/buttonbox.h>

#include <libicq2000/Client.h>
#include "main.h"
#include "sstream_fix.h"

using std::ostringstream;
using std::endl;

AuthRespDialog::AuthRespDialog(Gtk::Window& parent, const ICQ2000::ContactRef& contact, AuthReqICQMessageEvent *ev)
  : Gtk::Dialog("Authorisation Response", parent), m_contact(contact),
    m_grant("Grant", 0), m_refuse("Refuse", 0),
    m_label("Enter your refusal message:", 0)
{
  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing (10);

  Gtk::VBox *vbox2 = manage( new Gtk::VBox() );
  vbox2->set_spacing(5);
  
  ostringstream ostr;
  ostr << contact->getNameAlias() << " is requesting authorisation." << endl
       << "You should grant or refuse the request." << endl;
       
  if (!ev->getMessage().empty()) {
    ostr << "Their request message is: " << endl << endl
         << ev->getMessage() << endl;
  }
  
  Gtk::Label *label = manage( new Gtk::Label( ostr.str(), 0 ) );
  label->set_justify(Gtk::JUSTIFY_FILL);
  label->set_line_wrap(true);
  vbox2->pack_start( *label );

  m_grant.signal_clicked().connect( SigC::slot( *this, &AuthRespDialog::grant_clicked_cb ) );
  vbox2->pack_start( m_grant );

  m_refuse.signal_clicked().connect( SigC::slot( *this, &AuthRespDialog::refuse_clicked_cb ) );
  Gtk::RadioButton::Group gp = m_grant.get_group();
  m_refuse.set_group( gp );
  vbox2->pack_start( m_refuse );
  m_grant.set_active(true);
  m_textview.set_editable(true);

  // by default grant is selected - no message for grant
  m_textview.set_sensitive(false);
  m_label.set_sensitive(false);
  vbox2->pack_start( m_label );

  vbox2->pack_start( m_textview );

  vbox->pack_start(*vbox2);

  add_button("Send Response", Gtk::RESPONSE_OK);
  add_button("Ignore", Gtk::RESPONSE_CANCEL);
  
  set_default_size(250,100);
  set_border_width(10);
  show_all();
}

void AuthRespDialog::grant_clicked_cb()
{
  m_textview.set_sensitive(false);
  m_label.set_sensitive(false);
}

void AuthRespDialog::refuse_clicked_cb()
{
  m_textview.set_sensitive(true);
  m_label.set_sensitive(true);
}

void AuthRespDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    ICQ2000::AuthAckEvent *ev = new ICQ2000::AuthAckEvent(m_contact, m_textview.get_buffer()->get_text(), m_grant.get_active());
    icqclient.SendEvent(ev);
  }

  delete this;
}

