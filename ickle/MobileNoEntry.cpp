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

#include "sstream_fix.h"

#include <gtk--/label.h>

using std::ostringstream;

MobileNoEntry::MobileNoEntry()
  : Gtk::Table( 4, 2 )
{
  Gtk::Label *label;

  label = manage( new Gtk::Label( "Country" ) );
  attach( *label, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL );
  label = manage( new Gtk::Label( "Area Code" ) );
  attach( *label, 2, 3, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL );
  label = manage( new Gtk::Label( "Number" ) );
  attach( *label, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL );

  label = manage( new Gtk::Label( "+" ) );
  attach( *label, 0, 1, 1, 2, GTK_FILL, GTK_FILL );  
  m_country.set_max_length(3);
  attach( m_country, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL );
  m_areacode.set_max_length(5);
  attach( m_areacode, 2, 3, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL );
  m_number.set_max_length(10);
  attach( m_number, 3, 4, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL );
}

string MobileNoEntry::get_text() const {
  ostringstream ostr;
  ostr << m_country.get_text()
       << m_areacode.get_text()
       << m_number.get_text();
  return ostr.str();
}
