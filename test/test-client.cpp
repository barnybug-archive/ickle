
#include <iostream>

#include "Client.h"
#include "events.h"
#include "constants.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

using ICQ2000::ConnectedEvent;
using ICQ2000::DisconnectedEvent;
using ICQ2000::LogEvent;
using ICQ2000::MessageEvent;
using ICQ2000::NormalMessageEvent;
using ICQ2000::SMSMessageEvent;
using ICQ2000::SMSResponseEvent;
using ICQ2000::SMSReceiptEvent;
using ICQ2000::ContactListEvent;
using ICQ2000::StatusChangeEvent;

using namespace std;

/*
 * Test Client class
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

class TestClient : public Object {
 private:
  ICQ2000::Client icqclient;
  
 public:
  TestClient(unsigned int uin, const string& pass);

  void run();

  // -- Callbacks --
  void connected_cb(ConnectedEvent *c);
  void disconnected_cb(DisconnectedEvent *c);
  bool message_cb(MessageEvent *c);
  void logger_cb(LogEvent *c);
  void contactlist_cb(ContactListEvent *c);
};

TestClient::TestClient(unsigned int uin, const string& pass)
  : icqclient(uin, pass) {

  // set up Callbacks
  icqclient.connected.connect(slot(this,&TestClient::connected_cb));
  icqclient.disconnected.connect(slot(this,&TestClient::disconnected_cb));
  icqclient.messaged.connect(slot(this,&TestClient::message_cb));
  icqclient.logger.connect(slot(this,&TestClient::logger_cb));
  icqclient.contactlist.connect(slot(this,&TestClient::contactlist_cb));
}

void TestClient::run() {

  icqclient.Connect();

  while(1) {
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    int fd = icqclient.getSocketHandle();
    if (fd != -1) FD_SET(fd, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    int ret = select(fd+1, &rfds, NULL, NULL, &tv);
    if (ret) icqclient.Poll();
    else cout << "test-client: Patiently waiting.." << endl;
  }

  icqclient.Disconnect();
}

void TestClient::connected_cb(ConnectedEvent *c) {
  cout << "test-client: Connected" << endl;
}

void TestClient::disconnected_cb(DisconnectedEvent *c) {
  if (c->getReason() == DisconnectedEvent::REQUESTED) {
    cout << "test-client: Disconnect as requested" << endl;
  } else {
    cout << "test-client: Problem connecting: ";
    switch(c->getReason()) {
    case DisconnectedEvent::FAILED_LOWLEVEL:
      cout << "Socket problems";
      break;
    case DisconnectedEvent::FAILED_BADUSERNAME:
      cout << "Bad Username";
      break;
    case DisconnectedEvent::FAILED_TURBOING:
      cout << "Turboing";
      break;
    case DisconnectedEvent::FAILED_BADPASSWORD:
      cout << "Bad Password";
      break;
    case DisconnectedEvent::FAILED_MISMATCH_PASSWD:
      cout << "Username and Password did not match";
      break;
    case DisconnectedEvent::FAILED_UNKNOWN:
      cout << "Unknown";
      break;
    }
    cout << endl;
  }
}

bool TestClient::message_cb(MessageEvent *c) {

  if (c->getType() == MessageEvent::Normal) {

    NormalMessageEvent *msg = static_cast<NormalMessageEvent*>(c);
    cout << "test-client: Message received: " << msg->getMessage() << endl;

    ostringstream ostr;
    ostr << "Re: " << msg->getMessage() << endl << "I don't care!" << endl;
    NormalMessageEvent mv(c->getContact(), ostr.str());
    icqclient.SendEvent( &mv );

  } else if (c->getType() == MessageEvent::SMS) {

    SMSMessageEvent *smsmsg = static_cast<SMSMessageEvent*>(c);
    cout << "test-client: SMS received: from: " << smsmsg->getSender() << ": " << smsmsg->getMessage() << endl;
    if (smsmsg->getMessage() == "reply to me") {
      SMSMessageEvent sv(c->getContact(), "Well, here I am", true);
      icqclient.SendEvent( &sv );
    }

  } else if (c->getType() == MessageEvent::SMS_Response) {

    SMSResponseEvent *rsp = static_cast<SMSResponseEvent*>(c);
    if (rsp->deliverable()) {
      cout << "test-client: SMS delivered successfully" << endl;
    } else {
      cout << "test-client: SMS delivery failed: " << rsp->getErrorParam() << endl;
    }

  } else if (c->getType() == MessageEvent::SMS_Receipt) {

    SMSReceiptEvent *rcpt = static_cast<SMSReceiptEvent*>(c);
    if (rcpt->delivered()) {
      cout << "test-client: SMS " << rcpt->getMessage() << " delivered" << endl;
    } else {
      cout << "test-client: SMS " << rcpt->getMessage() << " not delivered" << endl;
    }
  }

  return true;
}

void TestClient::logger_cb(LogEvent *c) {

  switch(c->getType()) {
  case LogEvent::INFO:
    cout << "[34m";
    break;
  case LogEvent::WARN:
    cout << "[31m";
    break;
  case LogEvent::PACKET:
    cout << "[32m";
    break;
  }

  cout << c->getMessage() << endl;
  cout << "[39m";
}

void TestClient::contactlist_cb(ContactListEvent *c) {
  if (c->getType() == ContactListEvent::StatusChange) {
    StatusChangeEvent *u = static_cast<StatusChangeEvent*>(c);
    cout << "User " << u->getUIN() << " went ";
    switch(u->getStatus()) {
    case STATUS_ONLINE:
      cout << "online";
      break;
    case STATUS_DND:
      cout << "DND";
      break;
    case STATUS_NA:
      cout << "NA";
      break;
    case STATUS_OCCUPIED:
      cout << "occupied";
      break;
    case STATUS_FREEFORCHAT:
      cout << "free for chat";
      break;
    case STATUS_AWAY:
      cout << "away";
      break;
    case STATUS_OFFLINE:
      cout << "offline";
      break;
    default:
      cout << "unknown";
    }
    cout << endl;

  }
}

int main(int argc, char *argv[]) {
  cout << "-- Client test --" << endl;

  TestClient icq_client(68065772, "sputnik12");
  icq_client.run();
}

