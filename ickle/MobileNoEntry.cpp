/*
 * MobileNoEntry
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

#include "MobileNoEntry.h"

#include "ickle.h"
#include "sstream_fix.h"

#include <gtkmm/label.h>

using std::string;
using std::ostringstream;

MobileNoEntry::MobileNoEntry()
  : Gtk::Table( 4, 2 )
{
  Gtk::Label *label;

  label = manage( new Gtk::Label( _("C'try"), 1.0, 0.5 ) );
  attach( *label, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
  label = manage( new Gtk::Label( _("Area"), 0.5, 0.5 ) );
  attach( *label, 2, 3, 0, 1, Gtk::FILL, Gtk::FILL );
  label = manage( new Gtk::Label( _("Number"), 0.0, 0.5 ) );
  attach( *label, 3, 4, 0, 1, Gtk::FILL, Gtk::FILL );

  label = manage( new Gtk::Label( "+" ) );
  attach( *label, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );  

  m_country.set_max_length(3);
  m_country.set_width_chars(3);
  m_country.signal_changed().connect( m_signal_changed.slot() );
  attach( m_country, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL );

  m_areacode.set_max_length(5);
  m_areacode.set_width_chars(5);
  m_areacode.signal_changed().connect( m_signal_changed.slot() );
  attach( m_areacode, 2, 3, 1, 2, Gtk::FILL, Gtk::FILL );

  m_number.set_max_length(10);
  m_number.set_width_chars(10);
  m_number.signal_changed().connect( m_signal_changed.slot() );
  attach( m_number, 3, 4, 1, 2, Gtk::FILL, Gtk::FILL );
}

Glib::ustring MobileNoEntry::get_text() const
{
  /* locale support - tenious */
  ostringstream ostr;
  ostr << m_country.get_text()
       << m_areacode.get_text()
       << m_number.get_text();
  return Glib::locale_to_utf8(ostr.str());
}

SigC::Signal0<void>& MobileNoEntry::signal_changed()
{
  return m_signal_changed;
}

