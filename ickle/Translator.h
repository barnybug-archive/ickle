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

#ifndef ICKLE_TRANSLATOR_H
#define ICKLE_TRANSLATOR_H

#include <string>

#include <libicq2000/Translator.h>

class IckleTranslator : public ICQ2000::CRLFTranslator
{

 protected:
  std::string get_contact_encoding(const ICQ2000::ContactRef& c);

 public:
  IckleTranslator();

  virtual std::string client_to_server(const std::string& str,
				       ICQ2000::Encoding en,
				       const ICQ2000::ContactRef& c);

  virtual std::string server_to_client(const std::string& str,
				       ICQ2000::Encoding en,
				       const ICQ2000::ContactRef& c);
};

#endif
