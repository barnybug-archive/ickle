/* $Id: Settings.h,v 1.17 2003-01-02 16:40:00 barnabygray Exp $
 * 
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
#include <map>
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include <stdexcept>

class Settings : public SigC::Object
{
 private:
  std::map<const std::string, std::string> m_map;
  bool m_utf8;
  
 public:
  Settings();
  
  void load(const std::string& filename) throw(std::runtime_error);
  void save(const std::string& filename) throw(std::runtime_error);
  
  std::string         getValueString       (const std::string& k);
  int            getValueInt          (const std::string& k);
  unsigned int   getValueUnsignedInt  (const std::string& k);
  unsigned short getValueUnsignedShort(const std::string& k);
  unsigned char  getValueUnsignedChar (const std::string& k);
  bool           getValueBool         (const std::string& k);

  void setValue(const std::string& k, const std::string& v);
  void setValue(const std::string& k, int v);
  void setValue(const std::string& k, unsigned int v);
  void setValue(const std::string& k, unsigned short v);
  void setValue(const std::string& k, unsigned char v);
  void setValue(const std::string& k, bool v);

  void defaultValueUnsignedInt  (const std::string& k, unsigned int dflt, unsigned int lower = 0, unsigned int upper = 0xffffffff);
  void defaultValueUnsignedShort(const std::string& k, unsigned short dflt, unsigned short lower = 0, unsigned short upper = 0xffff);
  void defaultValueUnsignedChar (const std::string& k, unsigned char dflt, unsigned char lower = 0, unsigned char upper = 0xff);
  void defaultValueBool         (const std::string& k, bool dflt);
  void defaultValueString       (const std::string& k, const std::string& dflt);

  bool exists(const std::string& k);

  void removeValue(const std::string& k);

  virtual void defaultSettings();

  static std::string Escape(const std::string& t);
  static std::string Unescape(const std::string& t);

  SigC::Signal1<void,const std::string&> settings_changed;
};

#endif

