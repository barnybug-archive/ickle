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

#include <iostream>
#include <fstream>
#include <sstream>

using std::ostringstream;
using std::istringstream;
using std::ifstream;
using std::ofstream;

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

      m_map[k] = Unescape(v);
    }
  }

  return true;
}

bool Settings::save(const string& filename) {
  ofstream of( filename.c_str(), ios::out | ios::trunc );
  if (!of) return false;
  hash_map<const string,string,_HashString>::iterator curr = m_map.begin();
  while (curr != m_map.end()) {
    of << (*curr).first << " = " << Escape((*curr).second) << endl;
    ++curr;
  }
  of.close();
  return true;
}

string Settings::Escape(const string& s) {
  ostringstream ostr;
  string::const_iterator curr = s.begin();
  while (curr != s.end()) {
    switch(*curr) {
    case '\n':
      ostr << "\\n";
      break;
    case '\r':
      ostr << "\\r";
      break;
    case '\\':
      ostr << "\\\\";
      break;
    default:
      ostr << (*curr);
    }
    ++curr;
  }

  return ostr.str();
}

string Settings::Unescape(const string& s) {
  ostringstream ostr;
  string::const_iterator curr = s.begin();
  while (curr != s.end()) {
    if (*curr == '\\') {
      ++curr;
      if (curr != s.end()) {
	switch(*curr) {
	case 'n':
	  ostr << "\n";
	  break;
	case 'r':
	  ostr << "\r";
	  break;
	case '\\':
	  ostr << "\\";
	  break;
	}
      } else {
	ostr << "\\";
      }
    } else {
      ostr << (*curr);
    }
    ++curr;
  }
  return ostr.str();
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


// mmm.. could template a lot of this, oh well.
unsigned int Settings::getValueUnsignedInt(const string& k, unsigned int dflt,
					 unsigned int lower, unsigned int upper) {
  unsigned int v;
  if ( exists(k) ) {
    istringstream istr(m_map[k]);
    istr >> v;
    if (v < lower) v = lower;
    if (v > upper) v = upper;
  } else {
    v = dflt;
  }
  return v;
}

unsigned short Settings::getValueUnsignedShort(const string& k, unsigned short dflt,
					 unsigned short lower, unsigned short upper) {
  unsigned short v;
  if ( exists(k) ) {
    istringstream istr(m_map[k]);
    istr >> v;
    if (v < lower) v = lower;
    if (v > upper) v = upper;
  } else {
    v = dflt;
  }
  return v;
}

unsigned char Settings::getValueUnsignedChar(const string& k, unsigned char dflt,
					     unsigned char lower, unsigned char upper) {
  unsigned char v;
  if ( exists(k) ) {
    istringstream istr(m_map[k]);
    unsigned int vi;
    istr >> vi;
    v = (unsigned char)vi;
    if (vi < lower) v = lower;
    if (vi > upper) v = upper;
  } else {
    v = dflt;
  }
  return v;
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

void Settings::setValue(const string& k, unsigned short v) {
  ostringstream ostr;
  ostr << v;
  m_map[k] = ostr.str();
}

void Settings::setValue(const string& k, unsigned char v) {
  ostringstream ostr;
  ostr << (unsigned int)v;
  m_map[k] = ostr.str();
}

bool Settings::exists(const string& k) {
  return (m_map.count(k) != 0);
}

void Settings::defaultSettings() {
  // virtual
}
