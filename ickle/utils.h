/* $Id $
 * 
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <glibmm/convert.h>

#include <time.h>

namespace Utils
{
  /* conversion of utf8->locale, with a fallback sequence */
  std::string locale_from_utf8_with_fallback(const Glib::ustring& utf8_string,
					     const Glib::ustring& fallback = "?");

  /* utf8 -> console (locale) ie. just a synonym for the above */
  std::string console(const Glib::ustring& utf8_string);

  /* conversion of locale->utf8, without an risk of exceptions being thrown */
  Glib::ustring locale_to_utf8(const Glib::ustring& utf8_string);

  /* determine is the string is valid utf-8 */
  bool is_valid_utf8(const std::string& str);

  /* determine if this encoding exists */
  bool is_valid_encoding(const std::string& encoding);

  /* printf-style formatting, in a locale independent way */
  std::string format_string(const char * fmt, ...);

  /* format time in ickle-wide standard way */
  Glib::ustring format_time(time_t t);

  /* IP address -> string */
  Glib::ustring format_IP(unsigned int ip);

  /* date -> string */
  Glib::ustring format_date(unsigned char day, unsigned char month, unsigned short year);
};

#endif /* UTILS_H */
