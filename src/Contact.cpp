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
    : m_status(STATUS_OFFLINE), m_invisible(false), m_seqnum(0xffff),
      m_icqcontact(false), m_mobilecontact(false) {
  }

  Contact::Contact(unsigned int uin)
    : m_uin(uin), m_status(STATUS_OFFLINE), m_invisible(false),
      m_alias(UINtoString(m_uin)), m_seqnum(0xffff),
      m_icqcontact(true), m_mobilecontact(false) { }

  Contact::Contact(const string& a, const string& m)
    : m_alias(a), m_mobileno(m), m_icqcontact(false), m_status(STATUS_OFFLINE),
      m_seqnum(0xffff), m_invisible(false),
      m_mobilecontact(true), m_uin(nextImaginaryUIN()) { }

  Contact::~Contact() {
    while (!m_pending_msgs.empty()) {
      delete m_pending_msgs.back();
      m_pending_msgs.pop_back();
    }

  }

  unsigned int Contact::getUIN() const { return m_uin; }

  string Contact::getStringUIN() const { return UINtoString(m_uin); }

  string Contact::getAlias() const { return m_alias; }

  Status Contact::getStatus() const { return m_status; }

  string Contact::getMobileNo() const { return m_mobileno; }

  unsigned int Contact::getExtIP() const { return m_ext_ip; }

  unsigned int Contact::getLanIP() const { return m_lan_ip; }

  unsigned short Contact::getExtPort() const { return m_ext_port; }

  unsigned short Contact::getLanPort() const { return m_lan_port; }

  unsigned char Contact::getTCPVersion() const { return m_tcp_version; }

  bool Contact::acceptAdvancedMsgs() const {
    return (m_tcp_version >= 7);
  }

  bool Contact::isInvisible() const { return m_invisible; }

  void Contact::setMobileNo(const string& mn) {
    m_mobileno = mn;
    if (!mn.empty()) m_mobilecontact = true;
    else m_mobilecontact = false;
  }

  void Contact::setAlias(const string& al) { m_alias = al; }

  void Contact::setStatus(Status st) { m_status = st; }

  void Contact::setInvisible(bool inv) { m_invisible = inv; }

  bool Contact::isICQContact() const { return m_icqcontact; }

  bool Contact::isMobileContact() const { return m_mobilecontact; }

  void Contact::setExtIP(unsigned int ip) { m_ext_ip = ip; }

  void Contact::setLanIP(unsigned int ip) { m_lan_ip = ip; }

  void Contact::setExtPort(unsigned short port) { m_ext_port = port; }

  void Contact::setLanPort(unsigned short port) { m_lan_port = port; }

  void Contact::setTCPVersion(unsigned char v) { m_tcp_version = v; }

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

  unsigned short Contact::nextSeqNum() {
    return --m_seqnum;
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
