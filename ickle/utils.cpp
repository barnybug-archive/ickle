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

#include "utils.h"

namespace Utils
{
  /* 
   * locale_from_utf8_with_fallback: comparative to
   * Glib::locale_from_utf8, but with the option of a fallback
   */
  std::string locale_from_utf8_with_fallback(const Glib::ustring& utf8_string,
					     const Glib::ustring& fallback)
  {
    std::string to_codeset, from_codeset;
    
    if (!Glib::get_charset(to_codeset))
    {
      // UTF-8 -> locale
      from_codeset = "UTF-8";
      try
      {
	return Glib::convert_with_fallback( utf8_string, to_codeset, from_codeset, fallback);
      }
      catch(Glib::ConvertError& e)
      {
	return "";
      }
    }
    else
    {
      // UTF-8 -> UTF-8
      return utf8_string;
    }
  }

  /*
   * locale_to_utf8 that doesn't risk throwing any exceptions
   */
  Glib::ustring locale_to_utf8(const Glib::ustring& utf8_string)
  {
    try
    {
      return Glib::locale_to_utf8(utf8_string);
    }
    catch(Glib::ConvertError& e)
    {
      return "";
    }
  }

}
