/**
 * libICQ2000 Client header
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

#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include <sigc++/signal_system.h>

#include "buffer.h"
#include "socket.h"
#include "TLV.h"
#include "SNAC.h"
#include "events.h"
#include "constants.h"
#include "Contact.h"
#include "ContactList.h"
#include "custom_marshal.h"

using namespace SigC;
using namespace std;

namespace ICQ2000 {

  // -- Status Codes Flags --
  const unsigned short STATUS_FLAG_ONLINE = 0x0000;
  const unsigned short STATUS_FLAG_AWAY = 0x0001;
  const unsigned short STATUS_FLAG_DND = 0x0002;
  const unsigned short STATUS_FLAG_NA = 0x0004;
  const unsigned short STATUS_FLAG_OCCUPIED = 0x0010;
  const unsigned short STATUS_FLAG_FREEFORCHAT = 0x0020;
  const unsigned short STATUS_FLAG_INVISIBLE = 0x0100;

  class Client {

   private:
    enum State { NOT_CONNECTED,
		 AUTH_AWAITING_CONN_ACK,
		 AUTH_AWAITING_AUTH_REPLY,
		 BOS_AWAITING_CONN_ACK,
		 BOS_AWAITING_LOGIN_REPLY,
		 BOS_LOGGED_IN
    } m_state;

    unsigned int m_uin;
    string m_password;

    string m_authorizerHostname;
    unsigned short m_authorizerPort;

    string m_bosHostname;
    unsigned short m_bosPort;

    unsigned short m_client_seq_num;
    Status m_status;
    
    ContactList m_contact_list;

    unsigned char *m_cookie_data;
    unsigned short m_cookie_length;

    TCPSocket m_serverSocket;

    Buffer m_recv;
    
    void Init();
    unsigned short NextSeqNum();

    void ConnectAuthorizer();
    void DisconnectAuthorizer();
    void ConnectBOS();
    void DisconnectBOS();
    void DisconnectInt();

    // ------------------ Signal dispatchers -----------------
    void SignalConnect();
    void SignalDisconnect(DisconnectedEvent::Reason r);
    void SignalMessage(MessageSNAC *snac);
    void SignalSrvResponse(SrvResponseSNAC *snac);
    void SignalLog(LogEvent::LogType type, const string& msg);
    void SignalUserOnline(BuddyOnlineSNAC *snac);
    void SignalUserOffline(BuddyOfflineSNAC *snac);
    void SignalUserAdded(Contact *c);
    void SignalUserRemoved(Contact *c);
    // ------------------ Outgoing packets -------------------

    void SendAuthReq();
    void SendCookie();
    void SendCapabilities();
    void SendLogin();
    void SendOfflineMessagesRequest();
    void SendOfflineMessagesACK();

    void Send(Buffer& b);

    unsigned int FLAPHeader(Buffer& b, unsigned char channel);
    void FLAPFooter(Buffer& b, unsigned int d);

    // ------------------ Incoming packets -------------------

    /**
     *  non-blocking receives a waiting packet
     *  and parses and handles it
     *  returns: whether a packet was handled
     */
    bool Recv();

    void Parse();
    void ParseCh1(Buffer& b, unsigned short seq_num);
    void ParseCh2(Buffer& b, unsigned short seq_num);
    void ParseCh3(Buffer& b, unsigned short seq_num);
    void ParseCh4(Buffer& b, unsigned short seq_num);

    // -------------------------------------------------------

    Contact* lookupICQ(unsigned int uin);
    Contact* lookupMobile(const string& m);

    /* Maps the Status enum code to the real uint16
     * value ICQ sends and vice versa
     */
    unsigned short MapStatusToICQStatus(Status st);
    Status MapICQStatusToStatus(unsigned short st);

   public:
    Client();
    Client(const unsigned int uin, const string& password);
    ~Client();
    
    void setUIN(unsigned int uin) { m_uin = uin; }
    unsigned int getUIN() { return m_uin; }
    void setPassword(const string& password) { m_password = password; }
    string getPassword() { return m_password; }

    // -- Signals --
    Signal1<void,ConnectedEvent*> connected;
    Signal1<void,DisconnectedEvent*> disconnected;
    Signal1<bool,MessageEvent*,StopOnTrueMarshal> messaged;
    Signal1<void,ContactListEvent*> contactlist;
    Signal1<void,LogEvent*> logger;
    // -------------

    // -- Signal Dispatchers --
    void SignalUserInfoChange(Contact *c);
    void SignalMessageQueueChanged(Contact *c);
    // ------------------------

    // -- Ping server --
    void PingServer();

    // -- Send calls --
    void SendEvent(MessageEvent *ev);
    
    // -- Set Status --
    void setStatus(const Status st);
    /* This can be called when connected to set the initial status
     * when you do connect, or can be called whilst connected to
     * change status
     */
    Status getStatus();

    // -- Contact List --
    void addContact(Contact& c);
    void removeContact(const unsigned int uin);
    Contact* getContact(const unsigned int uin);

    void setServerHostname(const string& host) { m_authorizerHostname = host; }
    string getServerHostname() { return m_authorizerHostname; }
    void setServerPort(const unsigned short& port) { m_authorizerPort = port; }
    unsigned short getServerPort() { return m_authorizerPort; }

    void Poll();

    // might be useful for select calls within client
    // or library, eg. gtkmm
    int getSocketHandle() { return m_serverSocket.getSocketHandle(); }

    /* The client should be free to call Connect()
     * whenever they want. If we are connected already
     * it's ignored, if we aren't and are not already
     * connecting it should start the proceedings
     * Returns the file descriptor for the socket,
     * so the client can select() on it
     */
    int Connect();

    /*
     * Likewise for Disconnect()
     */
    void Disconnect();

    /* isConnected() is a convenience for the
     * client, it should correspond exactly to ConnectedEvents
     * & DisconnectedEvents the client gets
     */
    bool isConnected();
    
  };
}

#endif
