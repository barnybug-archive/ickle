/*
 * Translator
 * Copyright (C) 2003 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#include "Translator.h"

#include "utils.h"
#include "main.h"
#include "Settings.h"

#include <glib.h>

#include <iostream>

IckleTranslator::IckleTranslator()
{ }

std::string IckleTranslator::client_to_server(const std::string& str,
					      ICQ2000::Encoding en,
					      const ICQ2000::ContactRef& c)
{
  std::string ret;
  std::string encoding = get_contact_encoding(c);

  g_return_val_if_fail( Utils::is_valid_utf8(str), "" );

  std::cout << "client_to_server (using " << encoding << ") : "
	    << Utils::console(str) << std::endl;

  if ( en == ICQ2000::ENCODING_CONTACT_LOCALE )
  {
    ret = Glib::convert_with_fallback( str, encoding, "UTF-8", "?" );
  }
  else if (en == ICQ2000::ENCODING_ISO_8859_1 )
  {
    ret = Glib::convert_with_fallback( str, "ISO-8859-1", "UTF-8", "?" );
  }
  else if (en == ICQ2000::ENCODING_UTF8)
  {
    ret = str;
  }
  
  return ret;
}

std::string IckleTranslator::server_to_client(const std::string& str,
					      ICQ2000::Encoding en,
					      const ICQ2000::ContactRef& c)
{
  std::string ret;
  std::string encoding = get_contact_encoding(c);

  std::cout << "server_to_client (using " << encoding << ") : ";

  if ( en == ICQ2000::ENCODING_CONTACT_LOCALE )
  {
    ret = Glib::convert_with_fallback( str, "UTF-8", encoding, "?" );
  }
  else if (en == ICQ2000::ENCODING_ISO_8859_1 )
  {
    ret = Glib::convert_with_fallback( str, "UTF-8", "ISO-8859-1", "?" );
  }
  else
  {
    g_return_val_if_fail( Utils::is_valid_utf8(str), "" );
    ret = str;
  }

  std::cout << Utils::console(ret) << std::endl;

  return ret;
}

bool IckleTranslator::is_contact_encoding(const ICQ2000::ContactRef& c)
{
  return (m_encoding_map.count( c->getUIN() ) > 0);
}

std::string IckleTranslator::get_contact_encoding(const ICQ2000::ContactRef& c)
{
  std::string default_encoding = g_settings.getValueString( "encoding" );
  if (!Utils::is_valid_encoding(default_encoding))
  {
    /* failing that, fallback on locale encoding */
    Glib::get_charset(default_encoding);
  }
  
  if ( m_encoding_map.count( c->getUIN() ) )
  {
    if (Utils::is_valid_encoding( m_encoding_map[c->getUIN()] ))
    {
      default_encoding = m_encoding_map[c->getUIN()];
    }
  }

  return default_encoding;
}

void IckleTranslator::set_contact_encoding(const ICQ2000::ContactRef& c, std::string encoding)
{
  m_encoding_map[c->getUIN()] = encoding;
}

void IckleTranslator::unset_contact_encoding(const ICQ2000::ContactRef& c)
{
  m_encoding_map.erase( c->getUIN() );
}

// ===========================================================================
//  IckleTranslatorProxy
// ===========================================================================

IckleTranslatorProxy::IckleTranslatorProxy(ICQ2000::Translator& t)
  : m_translator(t)
{ }

std::string IckleTranslatorProxy::client_to_server(const std::string& str,
						   ICQ2000::Encoding en,
						   const ICQ2000::ContactRef& c)
{
  return m_translator.client_to_server(str, en, c);
}

std::string IckleTranslatorProxy::server_to_client(const std::string& str,
						   ICQ2000::Encoding en,
						   const ICQ2000::ContactRef& c)
{
  return m_translator.server_to_client(str, en, c);
}
