/* $Id: MessageEvent.h,v 1.6 2003-05-26 15:52:27 barnabygray Exp $
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

#ifndef MESSAGEEVENT_H
#define MESSAGEEVENT_H

#include <time.h>

#include <string>

#include <libicq2000/Contact.h>

// ============================================================================
//  MessageEvents
// ============================================================================

/*
 * These may be duplicating a lot of the libicq2000 functionality, but
 * at this point in time I'm trying to make ickle more independent and
 * possible to include support for other IMs, so it warrants
 * implementing our own event classes client-side.
 */

/**
 *  Base type for an event on any service
 */
class MessageEvent {
 private:
  time_t m_time;
  
 public:
  enum ServiceType {
    ICQ
  };

  MessageEvent(time_t t);
  virtual ~MessageEvent() { }

  virtual ServiceType getServiceType() const = 0;
  time_t getTime() const;
  void setTime(time_t t);
};

/**
 *  Base type for all events from the ICQ Service
 */
class ICQMessageEvent : public MessageEvent {
 public:
  enum ICQMessageType {
    Normal,
    URL,
    SMS,
    SMS_Receipt,
    AuthReq,
    AuthAck,
    EmailEx,
    UserAdd,
    WebPager,
    FileTransfer
  };

 private:
  ICQ2000::ContactRef m_contact;
  // for the moment use libicq2000 contacts, will implement our own at
  // some point

  bool m_urgent, m_tocontactlist;
  bool m_direct, m_offline, m_multi;
  std::string m_away_message;
    
 public:
  ICQMessageEvent(time_t t, const ICQ2000::ContactRef& c);
  virtual ~ICQMessageEvent() { }

  virtual ICQMessageType getICQMessageType() const = 0;
  
  ServiceType getServiceType() const;
  ICQ2000::ContactRef getICQContact();

  bool isUrgent() const;
  void setUrgent(bool b);

  bool isToContactList() const;
  void setToContactList(bool b);

  bool isDirect() const;
  void setDirect(bool b);

  bool isMultiParty() const;
  void setMultiParty(bool b);

  bool isOffline() const;
  void setOffline(bool b);

  std::string getAwayMessage() const;
  void setAwayMessage(const std::string& msg);
};

/**
 *  An Normal message
 */
class NormalICQMessageEvent : public ICQMessageEvent {
 private:
  std::string m_message;
  unsigned int m_foreground, m_background;
    
 public:
  NormalICQMessageEvent(time_t t, const ICQ2000::ContactRef& c, const std::string& msg,
			unsigned int fg, unsigned int bg);

  std::string getMessage() const;
  ICQMessageType getICQMessageType() const;

  unsigned int getForeground() const;
  void setForeground(unsigned int f);

  unsigned int getBackground() const;
  void setBackground(unsigned int b);
};
  
/**
 *  An URL Message
 */
class URLICQMessageEvent : public ICQMessageEvent {
 private:
  std::string m_message, m_url;
    
 public:
  URLICQMessageEvent(time_t t, const ICQ2000::ContactRef& c, const std::string& msg, const std::string& url);

  std::string getMessage() const;
  std::string getURL() const;
  ICQMessageType getICQMessageType() const;
};
  
/**
 *  An SMS message
 */
class SMSICQMessageEvent : public ICQMessageEvent {
 private:
  std::string m_message;

 public:
  SMSICQMessageEvent(time_t t, const ICQ2000::ContactRef& c, const std::string& msg);

  std::string getMessage() const;
  ICQMessageType getICQMessageType() const;
};

/**
 *  An SMS (delivery) receipt
 */
class SMSReceiptICQMessageEvent : public ICQMessageEvent {
 private:
  std::string m_message, m_message_id, m_destination, m_submission_time, m_delivery_time;
  bool m_delivered;
    
 public:
  SMSReceiptICQMessageEvent(time_t t, const ICQ2000::ContactRef& c, const std::string& msg, const std::string& message_id,
			    const std::string& submission_time, const std::string& delivery_time, bool del);
    
  ICQMessageType getICQMessageType() const;
  std::string getMessage() const;
  std::string getMessageId() const;
  std::string getDestination() const;
  std::string getSubmissionTime() const;
  std::string getDeliveryTime() const;
  bool delivered() const;
};

/**
 *  An Authorisation Request
 */
class AuthReqICQMessageEvent : public ICQMessageEvent {
 private:
  std::string m_message;

 public:
  AuthReqICQMessageEvent(time_t t, const ICQ2000::ContactRef& c, const std::string& msg);

  std::string getMessage() const;
  ICQMessageType getICQMessageType() const;
};
  
/**
 *  An Authorisation Acknowledge (success/failure)
 */
class AuthAckICQMessageEvent : public ICQMessageEvent {
 private:
  std::string m_message;
  bool m_granted;

 public:
  AuthAckICQMessageEvent(time_t t, const ICQ2000::ContactRef& c, bool granted);
  AuthAckICQMessageEvent(time_t t, const ICQ2000::ContactRef& c, const std::string& msg, bool granted);

  std::string getMessage() const;
  ICQMessageType getICQMessageType() const;
  bool isGranted() const;
};

/**
 *  An E-mail Express message
 */
class EmailExICQMessageEvent : public ICQMessageEvent {
 private:
  std::string m_message;

 public:
  EmailExICQMessageEvent(time_t t, const ICQ2000::ContactRef& c, const std::string& msg);

  std::string getMessage() const;
  ICQMessageType getICQMessageType() const;
};

/**
 *  A Web Pager message
 */
class WebPagerICQMessageEvent : public ICQMessageEvent {
 private:
  std::string m_message;

 public:
  WebPagerICQMessageEvent(time_t t, const ICQ2000::ContactRef& c, const std::string& msg);

  std::string getMessage() const;
  ICQMessageType getICQMessageType() const;
};

/**
 *  A 'User Added You' message
 */
class UserAddICQMessageEvent : public ICQMessageEvent 
{
 public:
  UserAddICQMessageEvent(time_t t, const ICQ2000::ContactRef & c);

  ICQMessageType getICQMessageType() const;
};


namespace ICQ2000 {
  class FileTransferEvent;
}

/**
 *  A File Transfer request
 */
class FileTransferICQMessageEvent : public ICQMessageEvent 
{
 private:
  std::string m_message, m_description;
  unsigned int m_size;
  bool m_cancelled;
  ICQ2000::FileTransferEvent *m_ev;
  
 public:
  FileTransferICQMessageEvent(time_t t, const ICQ2000::ContactRef& c,
			      const std::string& msg, const std::string& desc,
			      unsigned int size, ICQ2000::FileTransferEvent *ev);

  ICQMessageType getICQMessageType() const;

  std::string getMessage() const;
  std::string getDescription() const;
  unsigned int getSize() const;
  bool isCancelled() const;
  void setCancelled(bool b);

  ICQ2000::FileTransferEvent* getEvent();
};
#endif

