/*
 * SNAC base classes
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

#include "SNAC-base.h"

namespace ICQ2000 {

  // ------------------ Abstract SNACs ---------------

  void InSNAC::Parse(Buffer& b) {
    b >> m_flags
      >> m_requestID;
    ParseBody(b);
  }

  unsigned short InSNAC::Flags() const {
    return m_flags;
  }

  unsigned int InSNAC::RequestID() const {
    return m_requestID;
  }

  void OutSNAC::Output(Buffer& b) const {
    OutputHeader(b);
    OutputBody(b);
  }

  void OutSNAC::OutputHeader(Buffer& b) const {
    b << Family();
    b << Subtype();
    b << Flags();
    b << RequestID();
  }

  // --------------- Raw SNAC ---------------------------

  RawSNAC::RawSNAC(unsigned short f, unsigned short t)
    : m_family(f), m_subtype(t) { }

  void RawSNAC::ParseBody(Buffer& b) {
    unsigned char c;
    while(b.beforeEnd()) {
      b >> c;
      m_data << c;
    }
  }

}

Buffer& operator<<(Buffer& b, const ICQ2000::OutSNAC& snac) { snac.Output(b); return b; }