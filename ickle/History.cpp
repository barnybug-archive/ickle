/*
 * History
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

#include "main.h"
#include "History.h"
#include "sstream_fix.h"

using ICQ2000::NormalMessageEvent;
using ICQ2000::URLMessageEvent;
using ICQ2000::SMSMessageEvent;

using std::endl;

History::History() { }

History::History(Contact *c) {
  ostringstream fn;

  m_contact = c;
  fn << CONTACT_DIR << c->getUIN() << ".history";
  m_filename = fn.str();
}

string History::getFilename() const { return m_filename; }

void History::log(MessageEvent *ev, bool received) {
  ofstream of( m_filename.c_str(), std::ios::out | std::ios::app );

  if (of) {

    if (ev->getType() == MessageEvent::Normal) {
      NormalMessageEvent *nev = static_cast<NormalMessageEvent*>(ev);

      of << "Type: Normal" << endl
	 << "Time: " << ev->getTime() << endl 
	 << "Direction: " << ( received ? "Received" : "Sent" ) << endl
	 << "Offline: " << ( nev->isOfflineMessage() ? "Yes" : "No" ) << endl
	 << "Multiparty: " << ( nev->isMultiParty() ? "Yes" : "No" ) << endl
	 << "Message: ";

      quote_output( of, nev->getMessage() );
      of << endl;
      
    } else if (ev->getType() == MessageEvent::URL) {
      URLMessageEvent *uev = static_cast<URLMessageEvent*>(ev);

      of << "Type: URL" << endl
	 << "Time: " << ev->getTime() << endl 
	 << "Direction: " << ( received ? "Received" : "Sent" ) << endl
	 << "Offline: " << ( uev->isOfflineMessage() ? "Yes" : "No" ) << endl
	 << "Message: ";

      quote_output( of, uev->getMessage() );
      of << "URL: ";
      quote_output( of, uev->getURL() );
      of << endl;
      
    } else if (ev->getType() == MessageEvent::SMS) {
      SMSMessageEvent *sev = static_cast<SMSMessageEvent*>(ev);
      of << "Type: SMS" << endl
	 << "Time: " << ev->getTime() << endl 
	 << "Direction: " << ( received ? "Received" : "Sent" ) << endl;
      if (!received) of << "Receipt: " << ( sev->getRcpt() ? "Yes" : "No" ) << endl;
      of << "Message: ";
      quote_output( of, sev->getMessage() );
      of << endl;
    }
  }

  of.close();
}

void History::quote_output(ostream& ostr, const string& text) {
  int a = 0, l;
  while ( (l = text.find( '\n', a )) != -1 ) {
    ostr << text.substr( a, l-a ) << "\\" << endl;
    a = l + 1;
  }
  ostr << text.substr( a ) << " " << endl;
}
