/*
 * SendAuthReqDialog
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

#include "SendAuthReqDialog.h"

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>

#include <libicq2000/Client.h>
#include "main.h"

SendAuthReqDialog::SendAuthReqDialog(Gtk::Window& parent, const ICQ2000::ContactRef& contact)
  : Gtk::Dialog("Send Authorisation Request", parent),
    m_contact(contact)
{
  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);

  Gtk::VBox *vbox2 = manage(new Gtk::VBox());
  vbox2->pack_start( *manage(new Gtk::Label("Enter your authorisation request message:", 0)) );
  m_textview.set_editable(true);
  vbox2->pack_start( m_textview, true, true, 10 );

  Gtk::Label *label = manage( new Gtk::Label("Note: Authorisation is not strictly speaking required "
					     "within the ICQ protocol (there are is no security/privacy behind it). "
					     "It is generally polite to ask for Authorisation though, if it is required. "));
  label->set_justify(Gtk::JUSTIFY_FILL);
  label->set_line_wrap(true);
  vbox2->pack_start( *label );

  vbox->pack_start (*vbox2, true, true);

  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  
  set_default_size(250,100);
  set_border_width(10);
  show_all();
}

void SendAuthReqDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    ICQ2000::AuthReqEvent *ev = new ICQ2000::AuthReqEvent(m_contact, m_textview.get_buffer()->get_text());
    icqclient.SendEvent(ev);
  }

  delete this;
}
