/*
 * Dir
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

#ifndef DIR_H
#define DIR_H

#include <list>
#include <string>

using std::string;
using std::list;

class Dir {
 private:
  list<string> m_list;
  bool m_dironly;

 public:
  typedef list<string>::iterator iterator;

  Dir();
  ~Dir();

  bool list(const string& s);
  void free();
  
  void setDirectoriesOnly(bool b);

  iterator begin();
  iterator end();
};

#endif