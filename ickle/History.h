/*
 * History
 * Logging and later loading of history
 *
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

#ifndef HISTORY_H
#define HISTORY_H

#include <string>

#include "events.h"

using ICQ2000::MessageEvent;

using std::string;

class History {
 private:
  string m_filename;

  void quote_output(ostream& ostr, const string& text);

 public:
  History();
  History(const string& filename);
  
  void setFilename(const string& filename);
  string getFilename() const;

  void log(MessageEvent *ev, bool received);
};

#endif

