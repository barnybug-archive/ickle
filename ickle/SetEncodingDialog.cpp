/*
 * SetEncodingDialog
 * Copyright (C) 2003 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#include "SetEncodingDialog.h"

#include <libicq2000/events.h>
#include <libicq2000/Client.h>

#include <gtkmm/stock.h>
#include <gtkmm/table.h>
#include <gtkmm/messagedialog.h>

#include "main.h"
#include "utils.h"
#include "ickle.h"
#include "ucompose.h"
#include "Translator.h"

SetEncodingDialog::SetEncodingDialog(Gtk::Window& parent, const ICQ2000::ContactRef& c)
  : Gtk::Dialog( _("Set contact character set"), parent), m_contact(c)
{
  Gtk::Label *label;

  set_position(Gtk::WIN_POS_CENTER);

  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  set_default_response(Gtk::RESPONSE_OK);

  // libicq2000 callbacks
  icqclient.contactlist.connect( this, &SetEncodingDialog::contactlist_cb );

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(5);

  label = manage( new Gtk::Label("", 0.0, 0.5) );
  label->set_markup( _("You can set a per-contact character set, if the contact is running in a "
		       "different locale from the global default you've set in Preferences.") );
  label->set_line_wrap(true);
  label->set_justify(Gtk::JUSTIFY_FILL);

  vbox->pack_start( * label );

  Gtk::Table *table = manage( new Gtk::Table(3, 1) );
  table->set_spacings(5);
  
  table->attach( * manage( new Gtk::Label( _("Character set"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );
  
  m_encoding.signal_changed().connect( SigC::slot( *this, &SetEncodingDialog::validate_encoding_cb ) );
  m_encoding.set_text( g_translator.get_contact_encoding(c) );
  m_encoding.set_activates_default(true);
  table->attach( m_encoding, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
  
  m_valid.set_markup( _("<b>Valid</b>") );
  table->attach( m_valid, 2, 3, 0, 1, Gtk::FILL, Gtk::FILL );

  vbox->pack_start( * table );

  label = manage( new Gtk::Label("", 0.0, 0.5) );
  label->set_markup( _("You should use a character set name supported by iconv. "
		       "If you are using GNU <tt>iconv</tt>, you should be able to list the available encodings "
		       "by running the command:\n"
		       "<tt>iconv --list</tt>\n\n") );
  label->set_line_wrap(true);
  label->set_justify(Gtk::JUSTIFY_FILL);

  vbox->pack_start( * label );

  set_border_width(10);
  show_all();
}

void SetEncodingDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    if (Utils::is_valid_encoding(m_encoding.get_text()))
    {
      /* set it */
      g_translator.set_contact_encoding(m_contact, m_encoding.get_text());

      delete this;
    }
    else
    {
      /* error dialog */
      Gtk::MessageDialog dialog (*this,
				 String::ucompose( _("\"%1\" is not a valid encoding on your system."), m_encoding.get_text()),
				 Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
      
      dialog.run();
    }
  }
  else
  {
    delete this;
  }
  
}

void SetEncodingDialog::validate_encoding_cb()
{
  m_valid.set_sensitive( Utils::is_valid_encoding(m_encoding.get_text() ) );
}

void SetEncodingDialog::contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  if (ev->getType() == ICQ2000::ContactListEvent::UserRemoved)
  {
    ICQ2000::UserRemovedEvent *cev = static_cast<ICQ2000::UserRemovedEvent*>(ev);
    ICQ2000::ContactRef c = cev->getContact();
    if (m_contact->getUIN() == c->getUIN())
      delete this;
  }
}
