/*
 * Dir.cpp
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

#include "Dir.h"

#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

Dir::Dir()
  : m_dironly(false) { }

Dir::~Dir() { }

void Dir::free() {
  m_list.clear();
}

bool Dir::list(const string& s) {
  glob_t gr;

  free();

  if ( glob( s.c_str(), 0, NULL, &gr ) != 0 || gr.gl_pathc == 0 ) return false;

  char **nextp;
  nextp = gr.gl_pathv;
  
  while (*nextp != NULL) {
    if (m_dironly) {
      struct stat buf;
      if ( stat(*nextp, &buf) == 0 &&
	   S_ISDIR( buf.st_mode ) )
	m_list.push_back( string(*nextp) );
    }
    else
      m_list.push_back( string(*nextp) );

    ++nextp;
  }

  globfree(&gr);

  return true;
}

void Dir::setDirectoriesOnly(bool b) { m_dironly = b; }

Dir::iterator Dir::begin() {
  return m_list.begin();
}

Dir::iterator Dir::end() {
  return m_list.end();
}
