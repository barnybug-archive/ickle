/*
 * Events
 * ------
 *
 * The library signals everything that happens to the program through
 * calling the signal listeners that have been connected to the Signal
 * dispatchers in Client.
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

#ifndef EVENTS_H
#define EVENTS_H

#include <time.h>
#include <string>

#include "constants.h"

using std::string;

namespace ICQ2000 {

  class Contact;

  class Event {
   protected:
    time_t m_time;

   public:
    Event();
    Event(time_t t);

    time_t getTime() const;
    void setTime(time_t t);
  };

  // ----------------- SocketEvent ---------------------

  class SocketEvent : public Event {
   private:
    int m_fd;

   public:
    SocketEvent(int fd);
    virtual ~SocketEvent() { }

    int getSocketHandle() const;

    enum Mode {
      READ      = 1 << 0,
      WRITE     = 1 << 1,
      EXCEPTION = 1 << 2
    };
  };

  class AddSocketHandleEvent : public SocketEvent {
   private:
    Mode m_mode;

   public:
    AddSocketHandleEvent(int fd, Mode m);
    
    Mode getMode() const;
    bool isRead() const;
    bool isWrite() const;
    bool isException() const;
  };

  class RemoveSocketHandleEvent : public SocketEvent {
   public:
    RemoveSocketHandleEvent(int fd);
  };

  // ----------------- ConnectedEvent ------------------

  class ConnectedEvent : public Event {
   public:
    ConnectedEvent::ConnectedEvent();
  };

  // ---------------- DisconnectedEvent ----------------

  class DisconnectedEvent : public Event {
   public:
    enum Reason {
      REQUESTED,
      FAILED_LOWLEVEL,
      FAILED_BADUSERNAME,
      FAILED_TURBOING,
      FAILED_BADPASSWORD,
      FAILED_MISMATCH_PASSWD,
      FAILED_DUALLOGIN,
      FAILED_UNKNOWN
    };

   private:
    Reason m_reason;

   public:
    DisconnectedEvent(Reason r);

    Reason getReason() const;
  };

  // ---------------- LogEvent -------------------------

  class LogEvent : public Event {
   public:
    enum LogType {
      WARN,
      INFO,
      PACKET
    };

   private:
    LogType m_type;
    string m_msg;

   public:
    LogEvent(LogType type, const string& msg);

    LogType getType() const;
    string getMessage() const;
  };

  // ---------------- ContactList Events ----------------

  class ContactListEvent : public Event {
   public:
    enum EventType {
      StatusChange,
      UserInfoChange,
      UserAdded,
      UserRemoved,
      MessageQueueChanged
    };
    
   protected:
    Contact *m_contact;

   public:
    ContactListEvent(Contact* c);
    virtual ~ContactListEvent();
    
    Contact* getContact() const;
    unsigned int getUIN() const;

    virtual EventType getType() const = 0;
  };

  class UserInfoChangeEvent : public ContactListEvent {
   public:
    UserInfoChangeEvent(Contact* c);
    EventType getType() const;
  };

  class UserAddedEvent : public ContactListEvent {
   public:
    UserAddedEvent(Contact *c);
    EventType getType() const;
  };

  class UserRemovedEvent : public ContactListEvent {
   public:
    UserRemovedEvent(Contact *c);
    EventType getType() const;
  };

  class MessageQueueChangedEvent : public ContactListEvent {
   public:
    MessageQueueChangedEvent(Contact *c);
    EventType getType() const;
  };
  
  class StatusChangeEvent : public ContactListEvent {
   private:
    Status m_status;
    
   public:
    StatusChangeEvent(Contact* contact, Status status);

    EventType getType() const;
    Status getStatus() const;
  };

  // ---------------- MessageEvent(s) -------------------

  class MessageEvent : public Event {
   protected:
    Contact* m_contact;

   public:
    enum MessageType {
      Normal,
      URL,
      SMS,
      SMS_Response,
      SMS_Receipt
    };

    MessageEvent(Contact* c);
    virtual ~MessageEvent();

    virtual MessageType getType() const = 0;
    Contact* getContact();
  };

  class NormalMessageEvent : public MessageEvent {
   private:
    string m_message;
    bool m_offline, m_multi, m_direct;
    unsigned int m_foreground, m_background;
    
   public:
    NormalMessageEvent(Contact* c, const string& msg);
    NormalMessageEvent(Contact* c, const string& msg, bool multi);
    NormalMessageEvent(Contact* c, const string& msg, time_t t, bool multi);
    NormalMessageEvent(Contact* c, const string& msg, unsigned int fg, unsigned int bg);

    string getMessage() const;
    MessageType getType() const;
    unsigned int getSenderUIN() const;
    bool isOfflineMessage() const;
    bool isMultiParty() const;
    bool isDirect() const;
    unsigned int getForeground() const;
    unsigned int getBackground() const;
    void setForeground(unsigned int f);
    void setBackground(unsigned int b);
  };
  
  class URLMessageEvent : public MessageEvent {
   private:
    string m_message, m_url;
    bool m_offline;
    
   public:
    URLMessageEvent(Contact* c, const string& msg, const string& url);
    URLMessageEvent(Contact* c, const string& msg, const string& url, time_t t);

    string getMessage() const;
    string getURL() const;
    MessageType getType() const;
    unsigned int getSenderUIN() const;
    bool isOfflineMessage() const;
  };
  
  class SMSMessageEvent : public MessageEvent {
   private:
    string m_message, m_source, m_sender, m_senders_network;
    bool m_rcpt;

   public:
    SMSMessageEvent(Contact* c, const string& msg, bool rcpt);
    SMSMessageEvent(Contact* c, const string& msg, const string& source,
		    const string& senders_network, const string& time);

    string getMessage() const;
    MessageType getType() const;
    string getSource() const;
    string getSender() const;
    string getSenders_network() const;
    bool getRcpt() const;
  };

  class SMSResponseEvent : public MessageEvent {
   private:
    string m_source, m_error_param, m_network;
    int m_error_id;
    bool m_deliverable;
    
   public:
    SMSResponseEvent(Contact* c, const string& source, int error_id, const string& error_param);
    SMSResponseEvent(Contact* c, const string& source, const string& network);

    MessageType getType() const;
    string getSource() const;
    bool deliverable() const;
    string getNetwork() const;
    int getErrorId() const;
    string getErrorParam() const;

  };

  class SMSReceiptEvent : public MessageEvent {
   private:
    string m_message, m_message_id, m_destination, m_submission_time, m_delivery_time;
    bool m_delivered;
    
   public:
    SMSReceiptEvent(Contact* c, const string& msg, const string& message_id,
		    const string& submission_time, const string& delivery_time, bool del);
    
    MessageType getType() const;
    string getMessage() const;
    string getMessageId() const;
    string getDestination() const;
    string getSubmissionTime() const;
    string getDeliveryTime() const;
    bool delivered() const;
  };

  // --------------------- NewUIN Event -----------------------------

  class NewUINEvent : public Event {
   private:
    unsigned int m_uin;
    bool m_success;       

   public:
    NewUINEvent(unsigned int uin, bool success=true);
    unsigned int getUIN() const;
    bool isSuccess() const;
  };

  class RateInfoChangeEvent : public Event {
   public:
    enum RateClass {
      RATE_CHANGE=1,
      RATE_WARNING,
      RATE_LIMIT,
      RATE_LIMIT_CLEARED
    };
    
   private:
    unsigned short m_code;	
    unsigned short m_rateclass;	
    unsigned int m_windowsize;
    unsigned int m_clear;
    unsigned int m_alert;
    unsigned int m_limit;
    unsigned int m_disconnect;
    unsigned int m_currentavg;
    unsigned int m_maxavg;

   public:
    RateInfoChangeEvent(unsigned short code, unsigned short rateclass, 
                        unsigned int windowsize,unsigned int clear,
                        unsigned int alert,unsigned int limit,
                        unsigned int disconnect,unsigned int currentavg,
                        unsigned int maxavg);
    
    unsigned short getCode() const { return m_code; }	
    unsigned short getRateClass() const { return m_rateclass; }	
    unsigned int getWindowSize() const { return m_windowsize; }
    unsigned int getClear() const { return m_clear; }
    unsigned int getAlert() const { return m_alert; }
    unsigned int getLimit() const { return m_limit; }
    unsigned int getDisconnect() const { return m_disconnect; }
    unsigned int getCurrentAvg() const { return m_currentavg; }
    unsigned int getMaxAvg() const { return m_maxavg; }
  };
}

#endif
