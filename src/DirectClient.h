/*
 * DirectClient
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

#ifndef DIRECTCLIENT_H
#define DIRECTCLIENT_H

#include <string>
#include <sstream>

#include <sigc++/signal_system.h>

#include <stdlib.h>

#include "socket.h"
#include "buffer.h"
#include "events.h"
#include "exceptions.h"
#include "ICQ.h"
#include "Contact.h"

#include "Translator.h"

using namespace std;
using namespace SigC;

namespace ICQ2000 {

  class DirectClient {
   private:
    enum State { WAITING_FOR_INIT,
		 WAITING_FOR_UIN_CONFIRMATION,
		 WAITING_FOR_INIT_ACK,
		 CONNECTED };

    State m_state;

    TCPSocket *m_socket;
    Buffer m_recv;

    Contact *m_contact;

    unsigned short m_tcp_version, m_revision;
    unsigned int m_remote_uin;
    unsigned char m_tcp_flags;
    unsigned short m_eff_tcp_version;

    unsigned int m_local_uin, m_local_inet_ip, m_session_id;
    unsigned short m_local_server_port;

    void Parse();
    void ParseInitPacket(Buffer &b);
    void ParseInitAck(Buffer &b, unsigned short length);
    void ParsePacket(Buffer& b);
    void ParsePacketV6(Buffer& b);
    void ParsePacketV7(Buffer& b);

    void SendInitAck();
    void SendInitPacket();
    void SendPacketAck(unsigned short seqnum, unsigned short subCommand);
    void Send(Buffer &b);
    
    bool Decrypt(Buffer& in, Buffer& out);
    void Encrypt(Buffer& in, Buffer& out);
    static unsigned char client_check_data[];
    Translator *m_translator;

   public:
    DirectClient(TCPSocket *sock, unsigned int uin, unsigned inet_ip, unsigned short server_port,Translator* translator);
    ~DirectClient();

    void Recv();

    // ------------------ Signal dispatchers -----------------
    void SignalLog(LogEvent::LogType type, const string& msg);
    void SignalMessageEvent(MessageEvent *ev);
    // ------------------  Signals ---------------------------
    Signal1<void,LogEvent*> logger;
    Signal1<void,MessageEvent*> messaged;

    unsigned int getUIN() const;
    unsigned int getIP() const;
    unsigned short getPort() const;

    void setContact(Contact* c);

  };

  class DirectClientException : exception {
   private:
    string m_errortext;
    
   public:
    DirectClientException();
    DirectClientException(const string& text);
    ~DirectClientException() throw() { }

    const char* what() const throw();
  };
  
  class DisconnectedException : DirectClientException {
   public:
    DisconnectedException(const string& text);
  };

  class UINConfirmationException : DirectClientException {
   public:
    UINConfirmationException();
  };

}

#endif
