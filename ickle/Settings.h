/*
 * Settings
 * Handle loading/parsing/saving of settings to
 * a configuration file
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

#include <config.h>

#ifdef HAVE_EXT_HASH_MAP
# include <ext/hash_map>
#elif HAVE_HASH_MAP
# include <hash_map>
#else
# error "hash_map not defined"
#endif

using std::string;
using std::hash_map;

class _HashString {
 public:
  size_t operator()(string const &str) const { return hash<char const *>()(str.c_str()); }
};
  
class Settings {
 private:
  hash_map<const string,string,_HashString> m_map;
  
 public:
  Settings();
  
  bool load(const string& filename);
  bool save(const string& filename);
  
  string getValueString(const string& k);
  int getValueInt(const string& k);
  unsigned int getValueUnsignedInt(const string& k, unsigned int dflt = 0,
				 unsigned int lower = 0, unsigned int upper = 0xffffffff);
  unsigned short getValueUnsignedShort(const string& k, unsigned short dflt = 0,
				 unsigned short lower = 0, unsigned short upper = 0xffff);
  unsigned char getValueUnsignedChar(const string& k, unsigned char dflt = 0,
				 unsigned char lower = 0, unsigned char upper = 0xff);

  void setValue(const string& k, const string& v);
  void setValue(const string& k, int v);
  void setValue(const string& k, unsigned int v);
  void setValue(const string& k, unsigned short v);
  void setValue(const string& k, unsigned char v);

  bool exists(const string& k);

  virtual void defaultSettings();

  static string Escape(const string& t);
  static string Unescape(const string& t);

};

#endif

