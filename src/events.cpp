/*
 * Events
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

#include "events.h"

#include "Contact.h"

namespace ICQ2000 {

  // --------------- Event ---------------------------

  Event::Event() {
    m_time = time(NULL);
  }

  Event::Event(time_t t) : m_time(t) { }
  
  time_t Event::getTime() const { return m_time; }

  void Event::setTime(time_t t) { m_time = t; }

  // --------------- Connnected Event ----------------

  ConnectedEvent::ConnectedEvent() { }

  // --------------- Disconnected Event --------------

  DisconnectedEvent::DisconnectedEvent(Reason r) : m_reason(r) { }

  DisconnectedEvent::Reason DisconnectedEvent::getReason() const { return m_reason; }
  
  // --------------- Log Event -----------------------

  LogEvent::LogEvent(LogType type, const string& msg)
    : m_type(type), m_msg(msg) { }

  LogEvent::LogType LogEvent::getType() const { return m_type; }

  string LogEvent::getMessage() const { return m_msg; }

  // --------------- ContactList Event ---------------

  ContactListEvent::ContactListEvent(Contact *c) { m_contact = c; }

  Contact *ContactListEvent::getContact() const { return m_contact; }
  unsigned int ContactListEvent::getUIN() const { return m_contact->getUIN(); }
    
  ContactListEvent::~ContactListEvent() { }

  // ----------------- StatusChange Event ----------------

  StatusChangeEvent::StatusChangeEvent(Contact* contact, Status st)
    : ContactListEvent(contact), m_status(st) { }
  
  ContactListEvent::EventType StatusChangeEvent::getType() const { return StatusChange; }
  Status StatusChangeEvent::getStatus() const { return m_status; }

  // ----------------- UserAdded Event -------------------

  UserAddedEvent::UserAddedEvent(Contact* contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType UserAddedEvent::getType() const { return UserAdded; }

  // ----------------- UserRemoved Event -------------------

  UserRemovedEvent::UserRemovedEvent(Contact* contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType UserRemovedEvent::getType() const { return UserRemoved; }

  // ----------------- UserInfoChange Event -------------------

  UserInfoChangeEvent::UserInfoChangeEvent(Contact* contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType UserInfoChangeEvent::getType() const { return UserInfoChange; }

  // ----------------- MessageQueueChangedEvent -------------------

  MessageQueueChangedEvent::MessageQueueChangedEvent(Contact* contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType MessageQueueChangedEvent::getType() const { return MessageQueueChanged; }

  // --------------- Message Event -------------------

  MessageEvent::MessageEvent(Contact *c) : m_contact(c) { }
  MessageEvent::~MessageEvent() { }

  Contact* MessageEvent::getContact() { return m_contact; }

  // ---------------- Normal Message ---------------------

  NormalMessageEvent::NormalMessageEvent(Contact* c, const string& msg)
    : MessageEvent(c), m_message(msg), m_offline(false) { }

  NormalMessageEvent::NormalMessageEvent(Contact* c, const string& msg, bool multi)
    : MessageEvent(c), m_message(msg), m_multi(multi), m_offline(false) { }

  NormalMessageEvent::NormalMessageEvent(Contact *c, const string& msg, time_t t, bool multi)
    : MessageEvent(c), m_message(msg), m_offline(true), m_multi(multi) {
    m_time = t;
  }

  MessageEvent::MessageType NormalMessageEvent::getType() const { return MessageEvent::Normal; }
  
  unsigned int NormalMessageEvent::getSenderUIN() const { return m_contact->getUIN(); }

  string NormalMessageEvent::getMessage() const { return m_message; }

  bool NormalMessageEvent::isOfflineMessage() const { return m_offline; }

  bool NormalMessageEvent::isMultiParty() const { return m_multi; }

  // ---------------- URL Message ---------------------

  URLMessageEvent::URLMessageEvent(Contact* c, const string& msg, const string& url)
    : MessageEvent(c), m_message(msg), m_url(url), m_offline(false) { }

  URLMessageEvent::URLMessageEvent(Contact *c, const string& msg, const string& url, time_t t)
    : MessageEvent(c), m_message(msg), m_url(url), m_offline(true) {
    m_time = t;
  }

  MessageEvent::MessageType URLMessageEvent::getType() const { return MessageEvent::URL; }
  
  unsigned int URLMessageEvent::getSenderUIN() const { return m_contact->getUIN(); }

  string URLMessageEvent::getMessage() const { return m_message; }

  string URLMessageEvent::getURL() const { return m_url; }

  bool URLMessageEvent::isOfflineMessage() const { return m_offline; }

  // ---------------- SMS Message ------------------------

  SMSMessageEvent::SMSMessageEvent(Contact *c, const string& msg, const string& source,
				   const string& senders_network, const string& time)
    : MessageEvent(c), m_message(msg), m_source(source),
      m_senders_network(senders_network) {
    // fix: m_time = time;
  }

  SMSMessageEvent::SMSMessageEvent(Contact *c, const string& msg, bool rcpt)
    : MessageEvent(c), m_message(msg), m_rcpt(rcpt) { }

  MessageEvent::MessageType SMSMessageEvent::getType() const { return MessageEvent::SMS; }
  
  string SMSMessageEvent::getMessage() const { return m_message; }
  string SMSMessageEvent::getSource() const { return m_source; }
  string SMSMessageEvent::getSender() const { return m_contact->getMobileNo(); }
  string SMSMessageEvent::getSenders_network() const { return m_senders_network; }
  bool SMSMessageEvent::getRcpt() const { return m_rcpt; }

  // ---------------- SMS Response -----------------------

  SMSResponseEvent::SMSResponseEvent(Contact *c, const string& source, int error_id, const string& error_param)
    : MessageEvent(c), m_source(source), m_deliverable(false),
      m_error_id(error_id), m_error_param(error_param) { }
  
  SMSResponseEvent::SMSResponseEvent(Contact *c, const string& source, const string& network)
    : MessageEvent(c), m_source(source), m_deliverable(true),
      m_network(network) { }
  
  MessageEvent::MessageType SMSResponseEvent::getType() const { return MessageEvent::SMS_Response; }
  string SMSResponseEvent::getSource() const { return m_source; }
  string SMSResponseEvent::getNetwork() const { return m_network; }
  int SMSResponseEvent::getErrorId() const { return m_error_id; }
  string SMSResponseEvent::getErrorParam() const { return m_error_param; }
  bool SMSResponseEvent::deliverable() const { return m_deliverable; }

  // ---------------- SMS Receipt ------------------------

  SMSReceiptEvent::SMSReceiptEvent(Contact *c, const string& msg, const string& message_id,
				   const string& submission_time, const string& delivery_time, bool del)
    : MessageEvent(c), m_message(msg), m_message_id(message_id),
      m_submission_time(submission_time), m_delivery_time(delivery_time), m_delivered(del) { }
    
  MessageEvent::MessageType SMSReceiptEvent::getType() const { return MessageEvent::SMS_Receipt; }

  string SMSReceiptEvent::getMessage() const { return m_message; }
  string SMSReceiptEvent::getMessageId() const { return m_message_id; }
  string SMSReceiptEvent::getDestination() const { return m_contact->getMobileNo(); }
  string SMSReceiptEvent::getSubmissionTime() const { return m_submission_time; }
  string SMSReceiptEvent::getDeliveryTime() const { return m_delivery_time; }
  bool SMSReceiptEvent::delivered() const { return m_delivered; }

}
