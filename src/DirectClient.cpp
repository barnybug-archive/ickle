/*
 * DirectClient
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

#include "DirectClient.h"

#include "sstream_fix.h"

using std::ostringstream;

namespace ICQ2000 {

  unsigned char DirectClient::client_check_data[] = {
    "As part of this software beta version Mirabilis is "
    "granting a limited access to the ICQ network, "
    "servers, directories, listings, information and databases (\""
    "ICQ Services and Information\"). The "
    "ICQ Service and Information may databases (\""
    "ICQ Services and Information\"). The "
    "ICQ Service and Information may\0"
  };

  DirectClient::DirectClient(TCPSocket *sock, unsigned int uin, unsigned inet_ip, unsigned short server_port, Translator* translator)
    : m_socket(sock), m_state(WAITING_FOR_INIT),
      m_local_uin(uin), m_local_inet_ip(inet_ip),
      m_local_server_port(server_port),m_translator(translator), 
      m_recv(translator) { }

  DirectClient::~DirectClient() {
    delete m_socket;
  }

  void DirectClient::Recv() {
    try {
      while ( m_socket->RecvNonBlocking(m_recv) ) {
	ostringstream ostr;
	ostr << "Received packet from " << IPtoString( m_socket->getRemoteIP() ) << ":" << m_socket->getRemotePort() << endl << m_recv << endl;
	SignalLog(LogEvent::PACKET, ostr.str());
	Parse();
      }
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed on recv: " << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      throw DisconnectedException( ostr.str() );
    } catch(ParseException e) {
      ostringstream ostr;
      ostr << "Failed parsing: " << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      throw DisconnectedException( ostr.str() );
    }
  }

  void DirectClient::SignalLog(LogEvent::LogType type, const string& msg) {
    LogEvent ev(type,msg);
    logger.emit(&ev);
  }

  void DirectClient::SignalMessageEvent(MessageEvent *ev) {
    messaged.emit(ev);
  }

  void DirectClient::Parse() {
    if (m_recv.empty()) return;

    unsigned short length;

    while (!m_recv.empty()) {
      m_recv.setPos(0);

      m_recv.setEndianness(Buffer::LITTLE);
      m_recv >> length;
      if (m_recv.remains() < length) return; // waiting for more of the packet

      Buffer sb(m_translator);
      m_recv.chopOffBuffer( sb, length+2 );

      if (m_state == WAITING_FOR_INIT) {
	ParseInitPacket(sb);
	m_state = WAITING_FOR_UIN_CONFIRMATION;
	throw UINConfirmationException();
      } else if (m_state == WAITING_FOR_INIT_ACK) {
	ParseInitAck(sb);
	if (m_eff_tcp_version == 7)
	  m_state = WAITING_FOR_INIT2; // v7 has an extra stage of handshaking
	else
	  m_state = CONNECTED;          // v5 is done handshaking now
      } else if (m_state == WAITING_FOR_INIT2) {
	// This is a V7 only packet
	ParseInit2(sb);
	m_state = CONNECTED;
      } else if (m_state == CONNECTED) {
	ParsePacket(sb);
      }


      if (sb.beforeEnd()) {
	/* we assert that parsing code eats uses all data
	 * in the FLAP - seems useful to know when they aren't
	 * as it probably means they are faulty
	 */
	ostringstream ostr;
	ostr  << "Buffer pointer not at end after parsing packet was: 0x" << hex << sb.pos()
	      << " should be: 0x" << sb.size() << endl;
	SignalLog(LogEvent::WARN, ostr.str());
      }
      
    }
    
  }

  void DirectClient::SendInitPacket() {
    Buffer b(m_translator);
    b.setEndianness(Buffer::LITTLE);
    if (m_eff_tcp_version == 6) b << (unsigned short)0x002c; // length
    else if (m_eff_tcp_version == 7) b << (unsigned short)0x0030;

    b << (unsigned char)0xff;    // start byte
    b << (unsigned short)0x0007; // tcp version
    b << (unsigned short)0x0027; // revision
    
    b << m_remote_uin;
    b << (unsigned short)0x0000;
    b << (unsigned int)m_local_server_port;

    b << m_local_uin;
    b.setEndianness(Buffer::BIG);
    //    b << m_local_inet_ip;
    b << m_socket->getLocalIP(); // need to fix - get external IP from what the server tells us
    b << m_socket->getLocalIP();
    b << (unsigned char)0x04;    // mode
    b.setEndianness(Buffer::LITTLE);
    b << (unsigned int)m_local_server_port;
    b << m_session_id;

    b << (unsigned int)0x00000050; // unknown
    b << (unsigned int)0x00000003; // unknown
    if (m_eff_tcp_version == 7) 
      b << (unsigned int)0x00000000; // unknown
    
    Send(b);
  }

  void DirectClient::ParseInitPacket(Buffer &b) {
    b.setEndianness(Buffer::LITTLE);
    unsigned short length;
    b >> length;

    unsigned char start_byte;
    b >> start_byte;
    if (start_byte != 0xff) throw ParseException("Init Packet didn't start with 0xff");
    
    b >> m_tcp_version;
    b >> m_revision;
    
    if (m_tcp_version <= 5) throw ParseException("Too old client < ICQ99");
    if (m_tcp_version == 6) m_eff_tcp_version = m_tcp_version;
    else m_eff_tcp_version = 7;

    unsigned int our_uin;
    b >> our_uin;

    // 00 00
    // xx xx       senders open port
    // 00 00
    b.advance(6);

    b >> m_remote_uin;
    
    // xx xx xx xx  senders external IP
    // xx xx xx xx  senders lan IP
    b.advance(8);

    b >> m_tcp_flags;

    // xx xx        senders port again
    // 00 00
    b.advance(4);

    // xx xx xx xx  session id
    b >> m_session_id;

    // 50 00 00 00  unknown 
    // 03 00 00 00  unknown
    b.advance(8);

    if (m_eff_tcp_version == 7) {
      b.advance(4); // 00 00 00 00  unknown
    }

  }

  void DirectClient::ParseInitAck(Buffer &b) {
    b.setEndianness(Buffer::LITTLE);
    unsigned short length;
    b >> length;
    if (length != 4) throw ParseException("Init Ack not as expected");

    unsigned int a;
    b >> a;       // should be 0x00000001 really
  }

  void DirectClient::ParseInit2(Buffer &b) {
    b.setEndianness(Buffer::LITTLE);
    unsigned short length;
    b >> length;

    unsigned char type;
    b >> type;
    if (type != 0x03) throw ParseException("Expecting V7 final handshake packet, received something else");
    if (length != 0x0021) throw ParseException("V7 final handshake packet incorrect length");

    unsigned int unknown;
    b >> unknown // 0x0000000a
      >> unknown; // 0x00000001 on genuine connections, otherwise some weird connections which we drop

    if (unknown != 0x00000001) throw DisconnectedException("Ignoring weird direct connection");
  }

  void DirectClient::ParsePacket(Buffer& b) {
    Buffer c(m_translator);
    if (!Decrypt(b, c)) throw ParseException("Decrypting failed");
    if (m_eff_tcp_version == 6) ParsePacketV6(c);
    else if (m_eff_tcp_version == 7) ParsePacketV7(c);
    else throw ParseException("Unknown ICQ communication version");
  }

  void DirectClient::ParsePacketV6(Buffer& b) {
    b.setEndianness(Buffer::LITTLE);
    unsigned short length;
    b >> length;

    // we should get the decrypted packet in
    unsigned int checksum, foreground, background;
    unsigned short command, seqnum, unknown, ackFlags, msgFlags, version;
    unsigned char subCommand, flags;
    unsigned char junk;
    string msg;
    ostringstream ostr;


    b >> checksum
      >> command
      >> unknown // 0x000e
      >> seqnum;

    b.advance(12); // unknown 3 ints

    b >> subCommand
      >> flags
      >> ackFlags
      >> msgFlags;
    
    b.UnpackUint16StringNull(msg);
    b.ServerToClient(msg);

    if (command == 0 || subCommand == 0) throw ParseException("Invalid TCP Packet");

    MessageEvent *ev;

    switch(command) {

    case V6_TCP_START:

      switch(subCommand) {
      case MSG_Type_Normal:
	b >> foreground
	  >> background;
	
	SendPacketAck(seqnum, subCommand);

	ev = new NormalMessageEvent(m_contact, msg, foreground, background);
	SignalMessageEvent(ev);
	break;

      default:
	ostr << "Unhandled TCP subCommand 0x" << hex << subCommand << endl;
	throw ParseException( ostr.str() );
      }

      break;

    case V6_TCP_ACK:
      break;

    default:
      ostr << "Unknown TCP Command received 0x" << command;
      throw ParseException( ostr.str() );
    }

    b >> junk
      >> version;
  }

  void DirectClient::ParsePacketV7(Buffer& b) {
    b.setEndianness(Buffer::LITTLE);
    unsigned short length;
    b >> length;

    cout << "V7 packet" << endl;
  }

  bool DirectClient::Decrypt(Buffer& in, Buffer& out) {

    if (m_eff_tcp_version >= 6) {
      // Huge *thanks* to licq for this code
    
      unsigned long hex, key, B1, M1;
      unsigned int i;
      unsigned char X1, X2, X3;
      unsigned int size = in.size()-2;
      
      in.setEndianness(Buffer::LITTLE);
      out.setEndianness(Buffer::LITTLE);

      unsigned short length;
      in >> length;
      out << length;

      unsigned int check;
      in >> check;
      out << check;

      // main XOR key
      key = 0x67657268 * size + check;
      
      for(i=4; i<(size+3)/4; i+=4) {
	hex = key + client_check_data[i&0xFF];
	
	out << (unsigned char)(in.UnpackChar() ^ (hex&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>8)&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>16)&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>24)&0xFF));
      }
      
      unsigned char c;
      while (in.remains()) {
	in >> c;
	out << c;
      }

      B1 = (out[6]<<24) | (out[8]<<16) | (out[6]<<8) | (out[8]<<0);
      
      // special decryption
      B1 ^= check;
      
      // validate packet
      M1 = (B1 >> 24) & 0xFF;
      if(M1 < 10 || M1 >= size) return false;

      X1 = out[M1+2] ^ 0xFF;
      if(((B1 >> 16) & 0xFF) != X1) return false;
      
      X2 = ((B1 >> 8) & 0xFF);
      if(X2 < 220) {
	X3 = client_check_data[X2] ^ 0xFF;
	if((B1 & 0xFF) != X3) return false;
      }
    }

    ostringstream ostr;
    ostr << "Decrypted Direct packet from "  << IPtoString( m_socket->getRemoteIP() ) << ":" << m_socket->getRemotePort() << endl << out << endl;
    SignalLog(LogEvent::PACKET, ostr.str());
      
    return true;
  }

  void DirectClient::Encrypt(Buffer& in, Buffer& out) {

    ostringstream ostr;
    ostr << "Unencrypted packet to "  << IPtoString( m_socket->getRemoteIP() ) << ":" << m_socket->getRemotePort() << endl << in << endl;
    SignalLog(LogEvent::PACKET, ostr.str());
      
    if (m_eff_tcp_version == 6 || m_eff_tcp_version == 7) {
      // Huge *thanks* to licq for this code
    
      unsigned long hex, key, B1, M1;
      unsigned int i, check;
      unsigned char X1, X2, X3;
      unsigned int size = in.size();
      
      in.setEndianness(Buffer::LITTLE);
      out.setEndianness(Buffer::LITTLE);

      out << (unsigned short)size;

      // calculate verification data
      M1 = (rand() % ((size < 255 ? size : 255)-10))+10;
      X1 = in[M1] ^ 0xFF;
      X2 = rand() % 220;
      X3 = client_check_data[X2] ^ 0xFF;

      B1 = (in[4]<<24)|(in[6]<<16)|(in[4]<<8)|(in[6]);

      // calculate checkcode
      check = (M1 << 24) | (X1 << 16) | (X2 << 8) | X3;
      check ^= B1;

      out << check;

      // main XOR key
      key = 0x67657268 * size + check;
      
      // XORing the actual data
      in.advance(4);
      for(i=4;i<(size+3)/4;i+=4){
	hex = key + client_check_data[i&0xFF];

	out << (unsigned char)(in.UnpackChar() ^ (hex&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>8)&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>16)&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>24)&0xFF));
      }

      unsigned char c;
      while (in.remains()) {
	in >> c;
	out << c;
      }

    }

  }

  void DirectClient::SendInitAck() {
    Buffer b(m_translator);
    b.setEndianness(Buffer::LITTLE);
    b << (unsigned short)0x0004;
    b << (unsigned int)0x00000001;
    Send(b);
  }

  void DirectClient::SendPacketAck(unsigned short seqnum, unsigned short subCommand) {
    Buffer b(m_translator);

    b.setEndianness(Buffer::LITTLE);
    b << (unsigned int)0x00000000 // checksum (filled in by Encrypt)
      << V6_TCP_ACK
      << (unsigned short)0x000e
      << seqnum
      << (unsigned int)0x00000000
      << (unsigned int)0x00000000
      << (unsigned int)0x00000000
      << subCommand
      << (unsigned short)0x0000 // status  ?
      << (unsigned short)0x0000 // msgtype ?
      << (unsigned short)0x0001 // message - empty
      << (unsigned char)0x00 // message null terminator
      << (unsigned int)0x00000000
      << (unsigned int)0xffffffff;
    Buffer c(m_translator);
    Encrypt(b,c);
    Send(c);
  }

  void DirectClient::Send(Buffer &b) {
    try {
      ostringstream ostr;
      ostr << "Sending packet to "  << IPtoString( m_socket->getRemoteIP() ) << ":" << m_socket->getRemotePort() << endl << b << endl;
      SignalLog(LogEvent::PACKET, ostr.str());
      m_socket->Send(b);
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed to send: " << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      throw DisconnectedException( ostr.str() );
    }
  }

  unsigned int DirectClient::getUIN() const { return m_remote_uin; }

  unsigned int DirectClient::getIP() const { return m_socket->getRemoteIP(); }

  unsigned short DirectClient::getPort() const { return m_socket->getRemotePort(); }

  void DirectClient::setContact(Contact* c) {
    m_contact = c;
    if (m_state == WAITING_FOR_UIN_CONFIRMATION) {
      SendInitAck();
      SendInitPacket();
      m_state = WAITING_FOR_INIT_ACK;
    }
  }

  DirectClientException::DirectClientException() { }
  DirectClientException::DirectClientException(const string& text) : m_errortext(text) { }

  const char* DirectClientException::what() const throw() { return m_errortext.c_str(); }

  DisconnectedException::DisconnectedException(const string& text) : DirectClientException(text) { }
  
  UINConfirmationException::UINConfirmationException() { }

}
