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
    unsigned int m_ext_ip, m_lan_ip;
    unsigned short m_ext_port, m_lan_port;

    static unsigned int imag_uin;
    
    // other fields
    unsigned short m_seqnum;

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
    unsigned int getExtIP() const;
    unsigned int getLanIP() const;
    unsigned short getExtPort() const;
    unsigned short getLanPort() const;
    unsigned char getTCPVersion() const;

    void setMobileNo(const string& mn);
    void setAlias(const string& al);
    void setStatus(Status st);
    void setInvisible(bool i);
    void setExtIP(unsigned int ip);
    void setLanIP(unsigned int ip);
    void setExtPort(unsigned short port);
    void setLanPort(unsigned short port);
    void setTCPVersion(unsigned char v);
    
    bool isICQContact() const;
    bool isMobileContact() const;

    unsigned short nextSeqNum();

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
