/*
 * ICBM Cookie
 * Simple 8 byte message cookie
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

#ifndef ICBMCOOKIE_H
#define ICBMCOOKIE_H

#include "buffer.h"

using namespace std;

namespace ICQ2000 {

  class ICBMCookie {
   private:
    static const unsigned int cookie_size = 8;

    unsigned char m_cookie[cookie_size];

   public:
    ICBMCookie();

    void Parse(Buffer& b);
    void Output(Buffer& b) const;
  };

}

Buffer& operator<<(Buffer& b, const ICQ2000::ICBMCookie& c);
Buffer& operator>>(Buffer& b, ICQ2000::ICBMCookie& c);

#endif