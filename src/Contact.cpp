/*
 * Contact
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

#include "Contact.h"

namespace ICQ2000 {

  Contact::Contact()
    : m_status(STATUS_OFFLINE),
      m_icqcontact(false), m_mobilecontact(false) {
  }

  Contact::Contact(unsigned int uin)
    : m_uin(uin), m_status(STATUS_OFFLINE),
      m_alias(UINtoString(m_uin)),
      m_icqcontact(true), m_mobilecontact(false) { }

  Contact::Contact(const string& a, const string& m)
    : m_alias(a), m_mobileno(m), m_icqcontact(false),
      m_mobilecontact(true), m_uin(nextImaginaryUIN()) { }

  Contact::~Contact() {
    while (!m_pending_msgs.empty()) {
      delete m_pending_msgs.front();
      m_pending_msgs.pop_back();
    }

  }

  unsigned int Contact::getUIN() const { return m_uin; }

  string Contact::getStringUIN() const { return UINtoString(m_uin); }

  string Contact::getAlias() const { return m_alias; }

  Status Contact::getStatus() const { return m_status; }

  string Contact::getMobileNo() const { return m_mobileno; }

  void Contact::setMobileNo(const string& mn) {
    m_mobileno = mn;
    if (!mn.empty()) m_mobilecontact = true;
    else m_mobilecontact = false;
  }

  void Contact::setAlias(const string& al) { m_alias = al; }

  void Contact::setStatus(Status st) { m_status = st; }

  bool Contact::isICQContact() const { return m_icqcontact; }

  bool Contact::isMobileContact() const { return m_mobilecontact; }

  unsigned int Contact::numberPendingMessages() const { return m_pending_msgs.size(); }

  void Contact::addPendingMessage(MessageEvent* e) { return m_pending_msgs.push_back(e); }

  MessageEvent *Contact::getPendingMessage() const { return m_pending_msgs.front(); }

  void Contact::erasePendingMessage(MessageEvent* e) {
    list<MessageEvent*>::iterator curr = m_pending_msgs.begin();
    while (curr != m_pending_msgs.end()) {
      if (*curr == e) {
	m_pending_msgs.erase(curr);
	delete e;
	break;
      }
      ++curr;
    }
  }

  string Contact::UINtoString(unsigned int uin) {
    ostringstream ostr;
    ostr << uin;
    return ostr.str();
  }

  unsigned int Contact::StringtoUIN(const string& s) {
    istringstream istr(s);
    unsigned int uin = 0;
    istr >> uin;
    return uin;
  }

  unsigned int Contact::imag_uin = 0;
  
  unsigned int Contact::nextImaginaryUIN() {
    return (--imag_uin);
  }

}
  
