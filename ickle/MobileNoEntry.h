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

#ifndef MOBILENOENTRY_H
#define MOBILENOENTRY_H

#include <gtk--/widget.h>
#include <gtk--/entry.h>
#include <gtk--/label.h>
#include <gtk--/table.h>
#include <gtk--/box.h>

#include <string>
#include <sstream>

using namespace std;

class MobileNoEntry : public Gtk::Table {
 private:
  Gtk::Entry m_country, m_areacode, m_number;

 public:
  MobileNoEntry();

  string get_text() const;
};

#endif




