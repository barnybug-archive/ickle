/*
 * FindTextDialog
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 * Copyright (C) 2003 Christian Borntraeger <linux@borntraeger.net>.
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

#include "FindTextDialog.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/main.h>
#include <gtkmm/stock.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/entry.h>

#include "ickle.h"

using std::string;

FindTextDialog::FindTextDialog(Gtk::Window& parent, const Glib::ustring title,
    const Glib::ustring question, const Glib::ustring oldsearch="")
    : Gtk::Dialog(title, false, false)
{
  Gtk::VBox *vbox=get_vbox();

  m_case_sensitive.set_label(_("Case sensitive"));
  m_case_sensitive.set_active(false);

  vbox->pack_start(m_case_sensitive);

  m_tooltip.set_tip (m_search_text,_("What text are you looking for?"));
  m_search_text.set_text(oldsearch);
  m_search_text.set_activates_default(true);
  m_search_text.signal_changed().connect( SigC::slot( *this, &FindTextDialog::change_cb ) );
  vbox->pack_start(m_search_text);

  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  add_button( _("_Find Next"), RESPONSE_FIND);
  set_default_response(RESPONSE_FIND);

  set_position(Gtk::WIN_POS_CENTER);
  set_default_size( 250, 100 );
  set_border_width(10);
  set_transient_for(parent);
  show_all();
  m_search_text.grab_focus();
}

FindTextDialog::~FindTextDialog()
{
}

void FindTextDialog::change_cb() {
  signal_textsubmit.emit(m_search_text.get_text(), m_case_sensitive.get_active() );
}

void FindTextDialog::on_response(int response_id)
{
  switch (response_id) {
  case RESPONSE_FIND:	signal_textsubmit.emit(m_search_text.get_text(), m_case_sensitive.get_active() );
			break;
  default: 		break;
  }
}
