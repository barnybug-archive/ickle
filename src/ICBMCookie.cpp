/*
 * ICBM Cookie
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

#include "ICBMCookie.h"

namespace ICQ2000 {

  ICBMCookie::ICBMCookie() { }

  void ICBMCookie::Parse(Buffer& b) {
    b.Unpack(m_cookie, cookie_size);
  }

  void ICBMCookie::Output(Buffer& b) const {
    b.Pack(m_cookie, cookie_size);
  }
 
}

Buffer& operator<<(Buffer& b, const ICQ2000::ICBMCookie& c) { c.Output(b); return b; }

Buffer& operator>>(Buffer& b, ICQ2000::ICBMCookie& c) { c.Parse(b); return b; }
