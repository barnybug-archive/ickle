/*
 * Contact (model)
 * A contact on the contact list
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

#ifndef CONTACT_H
#define CONTACT_H

#include <list>
#include <string>
#include <sstream>

#include "events.h"
#include "constants.h"

using namespace std;

namespace ICQ2000 {

  class Contact {
  private:
    bool m_icqcontact;
    bool m_mobilecontact;

    list<MessageEvent*> m_pending_msgs;

    // static fields
    unsigned int m_uin;
    string m_alias;
    string m_mobileno;

    // dynamic fields - updated when they come online
    unsigned char m_tcp_version;
    Status m_status;
    bool m_invisible;

    static unsigned int imag_uin;
    
  public:
    Contact();
    Contact(unsigned int uin);
    Contact(const string& a, const string& m);

    ~Contact();

    unsigned int getUIN() const;
    string getStringUIN() const;
    string getAlias() const;
    Status getStatus() const;
    string getMobileNo() const;
    bool isInvisible() const;

    void setMobileNo(const string& mn);
    void setAlias(const string& al);
    void setStatus(Status st);
    void setInvisible(bool i);

    bool isICQContact() const;
    bool isMobileContact() const;

    unsigned int numberPendingMessages() const;

    void addPendingMessage(MessageEvent* e);
    MessageEvent *getPendingMessage() const;
    void erasePendingMessage(MessageEvent* e);

    static string UINtoString(unsigned int uin);
    static unsigned int StringtoUIN(const string& s);

    static unsigned int nextImaginaryUIN();

  };

}

#endif
