/* $Id: MessageEvent.cpp,v 1.1 2002-03-28 18:29:02 barnabygray Exp $
 *
 * Wrappers for ICQ Message Events.
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>,
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

#include "MessageEvent.h"

using std::string;

using ICQ2000::ContactRef;

// ============================================================================
//  MessageEvent
// ============================================================================

MessageEvent::MessageEvent(time_t t)
  : m_time(t) 
{ }

time_t MessageEvent::getTime() const
{
  return m_time;
}

void MessageEvent::setTime(time_t t)
{
  m_time = t;
}

// ============================================================================
//  ICQMessageEvent
// ============================================================================

ICQMessageEvent::ICQMessageEvent(time_t t, const ContactRef& c)
  : MessageEvent(t), m_contact(c), m_urgent(false), m_tocontactlist(false),
    m_direct(false), m_offline(false), m_multi(false)
{ }

MessageEvent::ServiceType ICQMessageEvent::getServiceType() const
{
  return ICQ;
}

ContactRef ICQMessageEvent::getICQContact()
{
  return m_contact;
}

bool ICQMessageEvent::isUrgent() const
{
  return m_urgent;
}

void ICQMessageEvent::setUrgent(bool b)
{
  m_urgent = b;
}

bool ICQMessageEvent::isToContactList() const
{
  return m_tocontactlist;
}

void ICQMessageEvent::setToContactList(bool b)
{
  m_tocontactlist = b;
}

bool ICQMessageEvent::isDirect() const
{
  return m_direct;
}

void ICQMessageEvent::setDirect(bool b)
{
  m_direct = b;
}

bool ICQMessageEvent::isMultiParty() const
{
  return m_multi;
}

void ICQMessageEvent::setMultiParty(bool b)
{
  m_multi = b;
}

bool ICQMessageEvent::isOffline() const
{
  return m_offline;
}

void ICQMessageEvent::setOffline(bool b)
{
  m_offline = b;
}


string ICQMessageEvent::getAwayMessage() const
{
  return m_away_message;
}

void ICQMessageEvent::setAwayMessage(const string& msg)
{
  m_away_message = msg;
}

// ============================================================================
//  NormalICQMessageEvent
// ============================================================================

NormalICQMessageEvent::NormalICQMessageEvent(time_t t, const ContactRef& c, const string& msg,
					     unsigned int fg, unsigned int bg)
  : ICQMessageEvent(t,c), m_message(msg), m_foreground(fg), m_background(bg)
{ }

string NormalICQMessageEvent::getMessage() const
{
  return m_message;
}

ICQMessageEvent::ICQMessageType NormalICQMessageEvent::getICQMessageType() const
{
  return Normal;
}

unsigned int NormalICQMessageEvent::getForeground() const
{
  return m_foreground;
}

void NormalICQMessageEvent::setForeground(unsigned int f)
{
  m_foreground = f;
}

unsigned int NormalICQMessageEvent::getBackground() const
{
  return m_background;
}

void NormalICQMessageEvent::setBackground(unsigned int b)
{
  m_background = b;
}
  
// ============================================================================
//  URLICQMessageEvent
// ============================================================================

URLICQMessageEvent::URLICQMessageEvent(time_t t, const ContactRef& c, const string& msg, const string& url)
  : ICQMessageEvent(t, c), m_message(msg), m_url(url)
{ }

string URLICQMessageEvent::getMessage() const
{
  return m_message;
}

string URLICQMessageEvent::getURL() const
{
  return m_url;
}

ICQMessageEvent::ICQMessageType URLICQMessageEvent::getICQMessageType() const
{
  return URL;
}
  
// ============================================================================
//  SMSICQMessageEvent
// ============================================================================

SMSICQMessageEvent::SMSICQMessageEvent(time_t t, const ContactRef& c, const string& msg)
  : ICQMessageEvent(t, c), m_message(msg)
{ }

string SMSICQMessageEvent::getMessage() const
{
  return m_message;
}

ICQMessageEvent::ICQMessageType SMSICQMessageEvent::getICQMessageType() const
{
  return SMS;
};

// ============================================================================
//  SMSReceiptICQMessageEvent
// ============================================================================

SMSReceiptICQMessageEvent::SMSReceiptICQMessageEvent(time_t t, const ContactRef& c, const string& msg,
						     const string& message_id, const string& submission_time,
						     const string& delivery_time, bool del)
  : ICQMessageEvent(t,c), m_message(msg), m_message_id(message_id), m_submission_time(submission_time),
    m_delivery_time(delivery_time), m_delivered(del)
{ }

ICQMessageEvent::ICQMessageType SMSReceiptICQMessageEvent::getICQMessageType() const
{
  return SMS_Receipt;
}

string SMSReceiptICQMessageEvent::getMessage() const
{
  return m_message;
}

string SMSReceiptICQMessageEvent::getMessageId() const
{
  return m_message_id;
}

string SMSReceiptICQMessageEvent::getDestination() const
{
  return m_destination;
}

string SMSReceiptICQMessageEvent::getSubmissionTime() const
{
  return m_submission_time;
}

string SMSReceiptICQMessageEvent::getDeliveryTime() const
{
  return m_delivery_time;
}

bool SMSReceiptICQMessageEvent::delivered() const
{
  return m_delivered;
}

// ============================================================================
//  AuthReqICQMessageEvent
// ============================================================================

AuthReqICQMessageEvent::AuthReqICQMessageEvent(time_t t, const ContactRef& c, const std::string& msg)
  : ICQMessageEvent(t,c), m_message(msg)
{ }

string AuthReqICQMessageEvent::getMessage() const
{
  return m_message;
}

ICQMessageEvent::ICQMessageType AuthReqICQMessageEvent::getICQMessageType() const
{
  return AuthReq;
}
  
// ============================================================================
//  AuthAckICQMessageEvent
// ============================================================================

AuthAckICQMessageEvent::AuthAckICQMessageEvent(time_t t, const ContactRef& c, bool granted)
  : ICQMessageEvent(t,c), m_granted(granted)
{ }

AuthAckICQMessageEvent::AuthAckICQMessageEvent(time_t t, const ContactRef& c, const string& msg, bool granted)
  : ICQMessageEvent(t,c), m_message(msg), m_granted(granted)
{ }

string AuthAckICQMessageEvent::getMessage() const
{
  return m_message;
}

bool AuthAckICQMessageEvent::isGranted() const
{
  return m_granted;
}

ICQMessageEvent::ICQMessageType AuthAckICQMessageEvent::getICQMessageType() const
{
  return AuthAck;
}

// ============================================================================
//  EmailExICQMessageEvent
// ============================================================================

EmailExICQMessageEvent::EmailExICQMessageEvent(time_t t, const ContactRef& c, const string& msg)
  : ICQMessageEvent(t,c), m_message(msg)
{ }

std::string EmailExICQMessageEvent::getMessage() const
{
  return m_message;
}

ICQMessageEvent::ICQMessageType EmailExICQMessageEvent::getICQMessageType() const
{
  return EmailEx;
}

