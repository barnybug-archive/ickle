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

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "ickle.h"

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

  std::string console(const Glib::ustring& utf8_string)
  {
    return locale_from_utf8_with_fallback(utf8_string);
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

  std::string format_string(const char * fmt, ...)
  {
    /* code partly borrowed from Linux printf manpage */

    /* Guess we need no more than 100 bytes. */
    int n, size = 100;
    std::string ret;
    char * p = new char[size];
    va_list ap;

    while (p)
    {
      /* Try to print in the allocated space. */
      va_start(ap, fmt);
      n = vsnprintf(p, size, fmt, ap);
      va_end(ap);
      
      /* If that worked, return the string. */
      if (n > -1 && n < size)
      {
	ret = p;
	break;
      }
      
      /* Else try again with more space. */
      if (n > -1)    /* glibc 2.1 */
	size = n+1; /* precisely what is needed */
      else           /* glibc 2.0 */
	size *= 2;  /* twice the old size */
      
      /* realloc - can't in C++ :-( */
      delete [] p;
      p = new char [size];
    }

    if (p != NULL)
      delete [] p;

    return ret;
  }


  std::string format_time(time_t t)
  {
    time_t now = time(NULL);
    struct tm now_tm = * (localtime(&now));
    struct tm tm = * (localtime(&t));

    char time_str[256];
    if (now - t > 86400 || now_tm.tm_mday != tm.tm_mday)
    {
      strftime(time_str, 255, "%d %b %Y %H:%M:%S", &tm);
    }
    else
    {
      strftime(time_str, 255, "%H:%M:%S", &tm);
    }

    return std::string(time_str);
  }

}
