/*
 * Settings
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

#include "Settings.h"

Settings::Settings() {
  defaultSettings();
}

bool Settings::load(const string& filename) {

  ifstream inf(filename.c_str());
  if (!inf) return false;

  while (inf) {
    string k, t, v;
    inf >> k >> t;
    if (t == "=") {
      while(inf.peek() == ' ') inf.ignore();
      getline(inf,v);

      m_map[k] = v;
    }
  }

  return true;
}

bool Settings::save(const string& filename) {
  ofstream of( filename.c_str(), ios::out | ios::trunc );
  if (!of) return false;
  hash_map<const string,string,_HashString>::iterator curr = m_map.begin();
  while (curr != m_map.end()) {
    of << (*curr).first << " = " << (*curr).second << endl;
    ++curr;
  }
  of.close();
  return true;
}

string Settings::getValueString(const string& k) {
  return m_map[k];
}

int Settings::getValueInt(const string& k) {
  istringstream istr(m_map[k]);
  int ret = 0;
  istr >> ret;
  return ret;
}

unsigned int Settings::getValueUnsignedInt(const string& k) {
  istringstream istr(m_map[k]);
  unsigned int ret = 0;
  istr >> ret;
  return ret;
}

void Settings::setValue(const string& k, const string& v) {
  m_map[k] = v;
}

void Settings::setValue(const string& k, int v) {
  ostringstream ostr;
  ostr << v;
  m_map[k] = ostr.str();
}

void Settings::setValue(const string& k, unsigned int v) {
  ostringstream ostr;
  ostr << v;
  m_map[k] = ostr.str();
}

bool Settings::exists(const string& k) {
  return (m_map.count(k) != 0);
}

void Settings::defaultSettings() {
  // virtual
}
