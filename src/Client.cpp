/*
 * libICQ2000 Client
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

#include "Client.h"

#include <sstream>
#include <algorithm>

#include "TLV.h"
#include "UserInfoBlock.h"

namespace ICQ2000 {

  Client::Client() : m_recv(&m_translator){
    Init();
  }

  Client::Client(const unsigned int uin, const string& password) : m_uin(uin), m_password(password), m_recv(&m_translator) {
    Init();
  }

  Client::~Client() {
    if (m_cookie_data)
      delete [] m_cookie_data;
    Disconnect();
  }

  void Client::Init() {
    m_authorizerHostname = "login.icq.com";
    m_authorizerPort = 5190;

    m_state = NOT_CONNECTED;
    
    m_cookie_data = NULL;
    m_cookie_length = 0;

    m_status = STATUS_OFFLINE;
    m_invisible = false;

    m_ext_ip = 0;
    m_buddy_uin = 0;
  }

  unsigned short Client::NextSeqNum() {
    return m_client_seq_num++;
  }

  void Client::ConnectAuthorizer(State state) {
    SignalLog(LogEvent::INFO, "Client connecting\n");
    try {
      /*
       * all sorts of SocketExceptions can be thrown
       * here - for
       * - sockets not being created
       * - DNS lookup failures
       */
      m_serverSocket.setRemoteHost(m_authorizerHostname.c_str());
      m_serverSocket.setRemotePort(m_authorizerPort);
      m_serverSocket.Connect();
      SignalAddSocket( m_serverSocket.getSocketHandle(), SocketEvent::READ );
    } catch(SocketException e) {
      // signal connection failure
      ostringstream ostr;
      ostr << "Failed to connect to Authorizer: " << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      SignalDisconnect(DisconnectedEvent::FAILED_LOWLEVEL);
      return;
    }
    

    // randomize sequence number
    srand(time(0));
    m_client_seq_num = (unsigned short)(0xFFFF*(rand()/(RAND_MAX+1.0)));

    m_state = state;
  }

  void Client::DisconnectAuthorizer() {
    SignalRemoveSocket( m_serverSocket.getSocketHandle() );
    m_serverSocket.Disconnect();
    m_state = NOT_CONNECTED;
  }

  void Client::ConnectBOS() {
    try {
      m_serverSocket.setRemoteHost(m_bosHostname.c_str());
      m_serverSocket.setRemotePort(m_bosPort);
      m_serverSocket.Connect();
      SignalAddSocket( m_serverSocket.getSocketHandle(), SocketEvent::READ );
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed to connect to BOS server: " << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      SignalDisconnect(DisconnectedEvent::FAILED_LOWLEVEL);
      return;
    }

    m_state = BOS_AWAITING_CONN_ACK;
  }

  void Client::DisconnectBOS() {
    SignalRemoveSocket( m_serverSocket.getSocketHandle() );
    m_serverSocket.Disconnect();
    SignalRemoveSocket( m_listenServer.getSocketHandle() );
    m_listenServer.Disconnect();
    DisconnectDirectConns();

    m_state = NOT_CONNECTED;
  }

  void Client::DisconnectDirectConns() {
    while ( !m_fdmap.empty() ) {
      delete (*m_fdmap.begin()).second;
      SignalRemoveSocket( (*m_fdmap.begin()).first );
      m_fdmap.erase( m_fdmap.begin() );
    }
  }

  void Client::DisconnectDirectConn(int fd) {
    SignalRemoveSocket(fd);
    delete m_fdmap[fd];
    m_fdmap.erase(fd);
  }

  // ------------------ Signal Dispatchers -----------------
  void Client::SignalConnect() {
    m_state = BOS_LOGGED_IN;
    ConnectedEvent ev;
    connected.emit(&ev);
  }

  void Client::SignalDisconnect(DisconnectedEvent::Reason r) {
    m_state = NOT_CONNECTED;
    DisconnectedEvent ev(r);
    disconnected.emit(&ev);

    // ensure all contacts return to Offline
    ContactList::iterator curr = m_contact_list.begin();
    while(curr != m_contact_list.end()) {
      if ( (*curr).getStatus() != STATUS_OFFLINE ) {
	(*curr).setStatus(STATUS_OFFLINE);
	StatusChangeEvent ev(&(*curr), (*curr).getStatus());
	contactlist.emit(&ev);
      }
      ++curr;
    }
  }

  void Client::SignalAddSocket(int fd, SocketEvent::Mode m) {
    AddSocketHandleEvent ev( fd, m );
    socket.emit(&ev);
  }

  void Client::SignalRemoveSocket(int fd) {
    RemoveSocketHandleEvent ev(fd);
    socket.emit(&ev);
  }

  void Client::SignalMessage(MessageSNAC *snac) {
    Contact *contact;
    MessageEvent *e = NULL;
    ICQSubType *st = snac->getICQSubType();
    if (st == NULL) return;

    if (st->getType() == MSG_Type_Normal) {
      NormalICQSubType *nst = static_cast<NormalICQSubType*>(st);
      
      contact = lookupICQ( nst->getSource() );
      e = new NormalMessageEvent(contact,
				 nst->getMessage(), nst->isMultiParty() );
      
      if (nst->isAdvanced()) SendAdvancedACK(snac);

    } else if (st->getType() == MSG_Type_URL) {
      URLICQSubType *ust = static_cast<URLICQSubType*>(st);
      
      contact = lookupICQ( ust->getSource() );
      e = new URLMessageEvent(contact,
			      ust->getMessage(),
			      ust->getURL());

      if (ust->isAdvanced()) SendAdvancedACK(snac);

    } else if (st->getType() == MSG_Type_SMS) {
      SMSICQSubType *sst = static_cast<SMSICQSubType*>(st);
      
      if (sst->getSMSType() == SMSICQSubType::SMS) {
	contact = lookupMobile(sst->getSender());
	e = new SMSMessageEvent(contact,
				sst->getMessage(),
				sst->getSource(),
				sst->getSenders_network(),
				sst->getTime());

      } else if (sst->getSMSType() == SMSICQSubType::SMS_Receipt_Success) {
	contact = lookupMobile(sst->getDestination());
	e = new SMSReceiptEvent(contact,
				sst->getMessage(),
				sst->getMessageId(),
				sst->getSubmissionTime(),
				sst->getDeliveryTime(),
				sst->delivered());
      }

    }

    if (e != NULL) {
      contact->addPendingMessage(e);
      if (messaged.emit(e)) contact->erasePendingMessage(e);
    }

  }

  void Client::SignalMessageEvent_cb(MessageEvent *ev) {
    Contact *contact = ev->getContact();
    contact->addPendingMessage(ev);
    if (messaged.emit(ev)) contact->erasePendingMessage(ev);
  }

  void Client::SignalSrvResponse(SrvResponseSNAC *snac) {
    if (snac->getType() == SrvResponseSNAC::OfflineMessagesComplete) {

      /* We are now meant to ACK this to say
       * the we have got the offline messages
       * and the server can dispose of storing
       * them
       */
      SendOfflineMessagesACK();

    } else if (snac->getType() == SrvResponseSNAC::OfflineMessage) {

      unsigned int uin = snac->getSenderUIN();
      Contact *contact = lookupICQ(uin);
      ICQSubType *st = snac->getICQSubType();

      MessageEvent *e = NULL;
      if (st->getType() == MSG_Type_Normal) {
	NormalICQSubType *nst = static_cast<NormalICQSubType*>(st);
	e = new NormalMessageEvent(contact,
				   nst->getMessage(),
				   snac->getTime(), nst->isMultiParty() );
      } else if (st->getType() == MSG_Type_URL) {
	URLICQSubType *ust = static_cast<URLICQSubType*>(st);
	e = new URLMessageEvent(contact,
				ust->getMessage(),
				ust->getURL(),
				snac->getTime());
      }
      
      if (e != NULL) {
	contact->addPendingMessage(e);
	if (messaged.emit(e)) contact->erasePendingMessage(e);
      }
      
    } else if (snac->getType() == SrvResponseSNAC::SMS_Error) {
      // mmm
    } else if (snac->getType() == SrvResponseSNAC::SMS_Response) {
      
      /*
       * Need to do SNAC Request ID caching to be able to associate these back
       * to the SMS and consequently the actual Mobile No
      MessageEvent *e = NULL;
      
      if (snac->deliverable()) {
	e = new SMSResponseEvent(snac->getSource(), snac->getNetwork());
      } else {
	if (snac->getErrorParam() != "DUPLEX RESPONSE")
	  e = new SMSResponseEvent(snac->getSource(), snac->getErrorId(), snac->getErrorParam());
	// ignore DUPLEX RESPONSE since I always get that
      }

      if (e != NULL) {
	contact->addPendingMessage(e);
	if (messaged.emit(e)) contact->erasePendingMessage(e);
	}

       *
       */

    } else if (snac->getType() == SrvResponseSNAC::SimpleUserInfo) {
      // update Contact
      if ( m_contact_list.exists( snac->getUIN() ) ) {
	Contact& c = m_contact_list[ snac->getUIN() ];
	c.setAlias( snac->getAlias() );
	c.setEmail( snac->getEmail() );
	c.setFirstName( snac->getFirstName() );
	c.setLastName( snac->getLastName() );
	UserInfoChangeEvent ev(&c);
	contactlist.emit(&ev);
      }
      
    } else if (snac->getType() == SrvResponseSNAC::RMainHomeInfo) {
      if ( m_buddy_uin > 0) {
        Contact& c = m_contact_list[ m_buddy_uin ];
        
        c.setMainHomeInfo( snac->getMainHomeInfo() );
        UserInfoChangeEvent ev(&c);
        contactlist.emit(&ev);
      }
    } else if (snac->getType() == SrvResponseSNAC::RHomepageInfo) {
      if ( m_buddy_uin > 0) {
        Contact& c = m_contact_list[ m_buddy_uin ];
        
        c.setHomepageInfo( snac->getHomepageInfo() );
        UserInfoChangeEvent ev(&c);
        contactlist.emit(&ev);
      }

    } else if (snac->getType() == SrvResponseSNAC::RAboutInfo) {
      if ( m_buddy_uin > 0) {
        Contact& c = m_contact_list[ m_buddy_uin ];
        m_buddy_uin = 0;
        c.setAboutInfo( snac->getAboutInfo() );
	UserInfoChangeEvent ev(&c);
        contactlist.emit(&ev);
      }
    }
  }
  
  void Client::SignalUINResponse(UINResponseSNAC *snac) {
    unsigned int uin = snac->getUIN();
    NewUINEvent e(uin);
    newuin.emit(&e);
  }

  void Client::SignalUINRequestError() {
    NewUINEvent e(0,false);
    newuin.emit(&e);
  }
  
  void Client::SignalRateInfoChange(RateInfoChangeSNAC *snac) {
    RateInfoChangeEvent e(snac->getCode(), snac->getRateClass(),
			  snac->getWindowSize(), snac->getClear(),
			  snac->getAlert(), snac->getLimit(),
			  snac->getDisconnect(), snac->getCurrentAvg(),
			  snac->getMaxAvg());
    rate.emit(&e);
  }

  void Client::SignalLog(LogEvent::LogType type, const string& msg) {
    LogEvent ev(type,msg);
    logger.emit(&ev);
  }
  
  void Client::SignalLog_cb(LogEvent *ev) {
    logger.emit(ev);
  }

  void Client::SignalUserOnline(BuddyOnlineSNAC *snac) {
    const UserInfoBlock& userinfo = snac->getUserInfo();
    if (m_contact_list.exists(userinfo.getUIN())) {
      Contact& c = m_contact_list[userinfo.getUIN()];
      c.setStatus( MapICQStatusToStatus(userinfo.getStatus()) );
      c.setInvisible( MapICQStatusToInvisible(userinfo.getStatus()) );
      c.setExtIP( userinfo.getExtIP() );
      c.setLanIP( userinfo.getLanIP() );
      c.setExtPort( userinfo.getExtPort() );
      c.setLanPort( userinfo.getLanPort() );
      c.setTCPVersion( userinfo.getTCPVersion() );
      StatusChangeEvent ev(&c, c.getStatus());
      contactlist.emit(&ev);
    } else {
      ostringstream ostr;
      ostr << "Received Status change for user not on contact list: " << userinfo.getUIN() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
    }
  }

  void Client::SignalUserOffline(BuddyOfflineSNAC *snac) {
    const UserInfoBlock& userinfo = snac->getUserInfo();
    if (m_contact_list.exists(userinfo.getUIN())) {
      Contact& c = m_contact_list[userinfo.getUIN()];
      c.setStatus(STATUS_OFFLINE);
      StatusChangeEvent ev(&c, c.getStatus());
      contactlist.emit(&ev);
    } else {
      ostringstream ostr;
      ostr << "Received Status change for user not on contact list: " << userinfo.getUIN() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
    }
  }

  // ------------------ Outgoing packets -------------------

  unsigned int Client::FLAPHeader(Buffer& b, unsigned char channel) {
    b << (unsigned char) 42;
    b << channel;
    b << NextSeqNum();
    b << (unsigned short) 0; // this is filled out later
    return b.size();
  }

  void Client::FLAPFooter(Buffer& b, unsigned int d) {
    unsigned short len;
    len = b.size() - d;
    b[d-2] = len >> 8;
    b[d-1] = len & 0xFF;
  }

  void Client::SendAuthReq() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x01);

    b << (unsigned int)0x00000001;

    b << ScreenNameTLV(Contact::UINtoString(m_uin))
      << PasswordTLV(m_password)
      << ClientProfileTLV("ICQ Inc. - Product of ICQ (TM).2000b.4.63.1.3279.85")
      << ClientTypeTLV(266)
      << ClientVersionMajorTLV(4)
      << ClientVersionMinorTLV(63)
      << ClientICQNumberTLV(1)
      << ClientBuildMajorTLV(3279)
      << ClientBuildMinorTLV(85)
      << LanguageTLV("en")
      << CountryCodeTLV("us");

    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Authorisation Request\n");
    Send(b);
  }

  void Client::SendNewUINReq() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x01);

    b << (unsigned int)0x00000001;
    FLAPFooter(b,d);
    Send(b);
    b.clear();

    d = FLAPHeader(b,0x02);
    UINRequestSNAC sn(m_password);
    b << sn;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending New UIN Request\n");
    Send(b);
  }
    
  void Client::SendCookie() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x01);

    b << (unsigned int)0x00000001;

    b << CookieTLV(m_cookie_data, m_cookie_length);

    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Login Cookie\n");
    Send(b);
  }
    
  void Client::SendCapabilities() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    CapabilitiesSNAC cs;
    b << cs;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Capabilities\n");
    Send(b);
  }

  void Client::SendSetUserInfo() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    SetUserInfoSNAC cs;
    b << cs;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Set User Info\n");
    Send(b);
  }

  void Client::SendRateInfoRequest() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    RequestRateInfoSNAC rs;
    b << rs;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Rate Info Request\n");
    Send(b);
  }
  
  void Client::SendRateInfoAck() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    RateInfoAckSNAC rs;
    b << rs;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Rate Info Ack\n");
    Send(b);
  }

  void Client::SendPersonalInfoRequest() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    PersonalInfoRequestSNAC us;
    b << us;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Personal Info Request\n");
    Send(b);
  }

  void Client::SendAddICBMParameter() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    MsgAddICBMParameterSNAC ms;
    b << ms;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Add ICBM Parameter\n");
    Send(b);
  }

  void Client::SendLogin() {
    Buffer b(&m_translator);
    unsigned int d;

    // startup listening server at this point, so we
    // know the listening port and ip
    m_listenServer.StartServer();
    SignalAddSocket( m_listenServer.getSocketHandle(), SocketEvent::READ );
    ostringstream ostr;
    ostr << "Server listening on " << IPtoString( m_serverSocket.getLocalIP() ) << ":" << m_listenServer.getPort() << endl;
    SignalLog(LogEvent::INFO, ostr.str());

    if (!m_contact_list.empty()) {
      d = FLAPHeader(b,0x02);
      AddBuddySNAC abs(m_contact_list);
      b << abs;
      FLAPFooter(b,d);
    }

    d = FLAPHeader(b,0x02);
    SetStatusSNAC sss(MapStatusToICQStatus(m_status, m_invisible));
    sss.setIP( m_serverSocket.getLocalIP() );
    sss.setPort( m_listenServer.getPort() );
    b << sss;
    FLAPFooter(b,d);

    //    d = FLAPHeader(b,0x02);
    //    SetIdleSNAC sis;
    //    b << sis;
    //    FLAPFooter(b,d);

    d = FLAPHeader(b,0x02);
    ClientReadySNAC crs;
    b << crs;
    FLAPFooter(b,d);

    d = FLAPHeader(b,0x02);
    SrvRequestOfflineSNAC ssnac(m_uin);
    b << ssnac;
    FLAPFooter(b,d);

    SignalLog(LogEvent::INFO, "Sending Contact List, Status, Client Ready and Offline Messages Request\n");
    Send(b);

    SignalConnect();
  }

  void Client::SendOfflineMessagesRequest() {
    Buffer b(&m_translator);
    unsigned int d;

    d = FLAPHeader(b,0x02);
    SrvRequestOfflineSNAC ssnac(m_uin);
    b << ssnac;
    FLAPFooter(b,d);

    SignalLog(LogEvent::INFO, "Sending Offline Messages Request\n");
    Send(b);
  }


  void Client::SendOfflineMessagesACK() {
    Buffer b(&m_translator);
    unsigned int d;

    d = FLAPHeader(b,0x02);
    SrvAckOfflineSNAC ssnac(m_uin);
    b << ssnac;
    FLAPFooter(b,d);

    SignalLog(LogEvent::INFO, "Sending Offline Messages ACK\n");
    Send(b);
  }

  void Client::SendAdvancedACK(MessageSNAC *snac) {
    ICQSubType *st = snac->getICQSubType();
    if (st == NULL || dynamic_cast<UINRelatedSubType*>(st) == NULL ) return;
    UINRelatedSubType *ust = dynamic_cast<UINRelatedSubType*>(snac->grabICQSubType());

    Buffer b(&m_translator);
    unsigned int d;
    
    d = FLAPHeader(b,0x02);
    
    MessageACKSNAC ssnac( snac->getICBMCookie(), ust );
    b << ssnac;
    FLAPFooter(b,d);

    SignalLog(LogEvent::INFO, "Sending Advanced ACK\n");
    Send(b);
  }

  void Client::Send(Buffer& b) {
    try {
      ostringstream ostr;
      ostr << "Sending packet to Server" << endl << b << endl;
      SignalLog(LogEvent::PACKET, ostr.str());
      m_serverSocket.Send(b);
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed to send: " << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      Disconnect();
    }
  }    

  // ------------------ Incoming packets -------------------

  void Client::RecvFromServer() {
    if (!m_serverSocket.connected()) return;

    try {
      while ( m_serverSocket.RecvNonBlocking(m_recv) ) {
	ostringstream ostr;
	ostr << "Received packet from Server" << endl << m_recv << endl;
	SignalLog(LogEvent::PACKET, ostr.str());
	Parse();
      }
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed on recv: " << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      Disconnect();
    }
  }

  void Client::Parse() {
    
    // -- FLAP header --

    unsigned char start_byte, channel;
    unsigned short seq_num, data_len;

    // process FLAP(s) in packet

    if (m_recv.empty()) return;

    while (!m_recv.empty()) {
      m_recv.setPos(0);

      m_recv >> start_byte;
      if (start_byte != 42) {
	m_recv.clear();
	SignalLog(LogEvent::WARN, "Invalid Start Byte on FLAP\n");
	return;
      }

      /* if we don't have at least six bytes we don't have enough
       * info to determine if we have the whole of the FLAP
       */
      if (m_recv.remains() < 5) return;
      
      m_recv >> channel;
      m_recv >> seq_num; // check sequence number - todo
      
      m_recv >> data_len;
      if (m_recv.remains() < data_len) return; // waiting for more of the FLAP

      /* Copy into another Buffer which is passed
       * onto the separate parse code that way
       * multiple FLAPs in one packet are split up
       */
      Buffer sb(&m_translator);
      m_recv.chopOffBuffer( sb, data_len+6 );
      sb.advance(6);

      // -- FLAP body --
      
      ostringstream ostr;
      
      switch(channel) {
      case 1:
	ParseCh1(sb,seq_num);
	break;
      case 2:
	ParseCh2(sb,seq_num);
	break;
      case 3:
	ParseCh3(sb,seq_num);
	break;
      case 4:
	ParseCh4(sb,seq_num);
	break;
      default:
	ostr << "FLAP on unrecognised channel 0x" << hex << (int)channel << endl;
	SignalLog(LogEvent::WARN, ostr.str());
	break;
      }

      if (sb.beforeEnd()) {
	/* we assert that parsing code eats uses all data
	 * in the FLAP - seems useful to know when they aren't
	 * as it probably means they are faulty
	 */
	ostringstream ostr;
	ostr  << "Buffer pointer not at end after parsing FLAP was: 0x" << hex << sb.pos()
	      << " should be: 0x" << sb.size() << endl;
	SignalLog(LogEvent::WARN, ostr.str());
      }
      
    }

  }

  void Client::ParseCh1(Buffer& b, unsigned short seq_num) {

    if (b.remains() == 4 && (m_state == AUTH_AWAITING_CONN_ACK || 
			     m_state == UIN_AWAITING_CONN_ACK)) {

      // Connection Acknowledge - first packet from server on connection
      unsigned int unknown;
      b >> unknown; // always 0x0001

      if (m_state == AUTH_AWAITING_CONN_ACK) {
	SendAuthReq();
	SignalLog(LogEvent::INFO, "Connection Acknowledge from server, sending Authorisation request\n");
	m_state = AUTH_AWAITING_AUTH_REPLY;
      } else if (m_state == UIN_AWAITING_CONN_ACK) {
	SendNewUINReq();
	SignalLog(LogEvent::INFO, "Connection Acknowledge from server, sending New UIN request\n");
	m_state = UIN_AWAITING_UIN_REPLY;
      }

    } else if (b.remains() == 4 && m_state == BOS_AWAITING_CONN_ACK) {

      SignalLog(LogEvent::INFO, "Connection Acknowledge from server, sending cookie\n");

      // Connection Ack, send the cookie
      unsigned int unknown;
      b >> unknown; // always 0x0001

      SendCookie();
      m_state = BOS_AWAITING_LOGIN_REPLY;

    } else {
      SignalLog(LogEvent::WARN, "Unknown packet received on channel 0x01\n");
    }

  }

  void Client::ParseCh2(Buffer& b, unsigned short seq_num) {
    InSNAC *snac;
    try {
      snac = ParseSNAC(b);
    } catch(ParseException e) {
      ostringstream ostr;
      ostr << "Problem parsing SNAC: " << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      return;
    }

    switch(snac->Family()) {
      
    case SNAC_FAM_GEN:
      switch(snac->Subtype()) {
      case SNAC_GEN_ServerReady:
	SignalLog(LogEvent::INFO, "Received Server Ready from server\n");
	SendCapabilities();
	break;
      case SNAC_GEN_RateInfo:
	SignalLog(LogEvent::INFO, "Received Rate Information from server\n");
	SendRateInfoAck();
	SendPersonalInfoRequest();
	SendAddICBMParameter();
	SendSetUserInfo();
	SendLogin();
	break;
      case SNAC_GEN_CapAck:
	SignalLog(LogEvent::INFO, "Received Capabilities Ack from server\n");
	SendRateInfoRequest();
	break;
      case SNAC_GEN_UserInfo:
	SignalLog(LogEvent::INFO, "Received User Info from server\n");
	HandleUserInfoSNAC(static_cast<UserInfoSNAC*>(snac));
	break;
      case SNAC_GEN_MOTD:
	SignalLog(LogEvent::INFO, "Received MOTD from server\n");
	//	SignalConnect();
	/* take the MOTD as the sign we
	 * are online proper
	 * - unfortunately they seemed to have stopped sending that
	 *   so this isn't the sign anymore.
	 */
	break;
      case SNAC_GEN_RateInfoChange:
	SignalLog(LogEvent::INFO, "Received Rate Info Change from server\n");
	SignalRateInfoChange(static_cast<RateInfoChangeSNAC*>(snac));
	break;
      }
      break;

    case SNAC_FAM_BUD:
      switch(snac->Subtype()) {
      case SNAC_BUD_Online:
	SignalLog(LogEvent::INFO, "Received Buddy Online from server\n");
	SignalUserOnline(static_cast<BuddyOnlineSNAC*>(snac));
	break;
      case SNAC_BUD_Offline:
	SignalLog(LogEvent::INFO, "Received Buddy Offline from server\n");
	SignalUserOffline(static_cast<BuddyOfflineSNAC*>(snac));
      }
      break;

    case SNAC_FAM_MSG:
      switch(snac->Subtype()) {
      case SNAC_MSG_Message:
	SignalLog(LogEvent::INFO, "Received Message from server\n");
	SignalMessage(static_cast<MessageSNAC*>(snac));
	break;
      }
      break;

    case SNAC_FAM_SRV:
      switch(snac->Subtype()) {
      case SNAC_SRV_Response:
	SignalLog(LogEvent::INFO, "Received Server Response from server\n");
	SignalSrvResponse(static_cast<SrvResponseSNAC*>(snac));
	break;
      }
      break;
    case SNAC_FAM_UIN:
      switch(snac->Subtype()) {
      case SNAC_UIN_Response:
	SignalLog(LogEvent::INFO, "Received UIN Response from server\n");
	SignalUINResponse(static_cast<UINResponseSNAC*>(snac));
	break;
      case SNAC_UIN_RequestError:
	SignalLog(LogEvent::INFO, "Received UIN Request Error from server\n");
	SignalUINRequestError();
	break;
      }
      break;
	
	
    } // switch(Family)

    if (dynamic_cast<RawSNAC*>(snac)) {
      ostringstream ostr;
      ostr << "Unknown SNAC packet received - Family: 0x" << hex << snac->Family()
	   << " Subtype: 0x" << snac->Subtype() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
    }

    delete snac;

  }

  void Client::ParseCh3(Buffer& b, unsigned short seq_num) {
    SignalLog(LogEvent::INFO, "Received packet on channel 0x03\n");
  }

  void Client::ParseCh4(Buffer& b, unsigned short seq_num) {
    if (m_state == AUTH_AWAITING_AUTH_REPLY || m_state == UIN_AWAITING_UIN_REPLY) {
      // An Authorisation Reply / Error
      TLVList tlvlist;
      tlvlist.Parse(b, TLV_ParseMode_Channel04, (short unsigned int)-1);

      if (tlvlist.exists(TLV_Cookie) && tlvlist.exists(TLV_Redirect)) {

	RedirectTLV *r = static_cast<RedirectTLV*>(tlvlist[TLV_Redirect]);
	ostringstream ostr;
	ostr << "Redirected to: " << r->getHost() << " port: " << dec << r->getPort() << endl;
	SignalLog(LogEvent::INFO, ostr.str());

	m_bosHostname = r->getHost();
	m_bosPort = r->getPort();

	// Got our cookie - yum yum :-)
	CookieTLV *t = static_cast<CookieTLV*>(tlvlist[TLV_Cookie]);
	m_cookie_length = t->Length();

	if (m_cookie_data) delete [] m_cookie_data;
	m_cookie_data = new unsigned char[m_cookie_length];

	memcpy(m_cookie_data, t->Value(), m_cookie_length);

	SignalLog(LogEvent::INFO, "Authorisation accepted\n");
	
	DisconnectAuthorizer();
	ConnectBOS();

      } else {
	// Problemo
	DisconnectedEvent::Reason st;

	if (tlvlist.exists(TLV_ErrorCode)) {
	  ErrorCodeTLV *t = static_cast<ErrorCodeTLV*>(tlvlist[TLV_ErrorCode]);
	  ostringstream ostr;
	  ostr << "Error logging in Error Code: " << t->Value() << endl;
	  SignalLog(LogEvent::WARN, ostr.str());
	  switch(t->Value()) {
	  case 0x01:
	    st = DisconnectedEvent::FAILED_BADUSERNAME;
	    break;
	  case 0x02:
	    st = DisconnectedEvent::FAILED_TURBOING;
	    break;
	  case 0x03:
	    st = DisconnectedEvent::FAILED_BADPASSWORD;
	    break;
	  case 0x05:
	    st = DisconnectedEvent::FAILED_MISMATCH_PASSWD;
	    break;
	  case 0x18:
	    st = DisconnectedEvent::FAILED_TURBOING;
	    break;
	  default:
	    st = DisconnectedEvent::FAILED_UNKNOWN;
	  }
	} else if (m_state == AUTH_AWAITING_AUTH_REPLY) {
	    SignalLog(LogEvent::WARN, "Error logging in, no error code given(?!)\n");
	    st = DisconnectedEvent::FAILED_UNKNOWN;
	  } 	
	DisconnectAuthorizer();
	SignalDisconnect(st); // signal client (error)
      }

    } else {

      TLVList tlvlist;
      tlvlist.Parse(b, TLV_ParseMode_Channel04, (short unsigned int)-1);

      DisconnectedEvent::Reason st;
      
      if (tlvlist.exists(TLV_DisconnectReason)) {
	DisconnectReasonTLV *t = static_cast<DisconnectReasonTLV*>(tlvlist[TLV_DisconnectReason]);
	  switch(t->Value()) {
	  case 0x0001:
	    st = DisconnectedEvent::FAILED_DUALLOGIN;
	    break;
	  default:
	    st = DisconnectedEvent::FAILED_UNKNOWN;
	  }

	} else {
	  SignalLog(LogEvent::WARN, "Unknown packet received on channel 4, disconnecting\n");
	  st = DisconnectedEvent::FAILED_UNKNOWN;
	}
	DisconnectBOS();
	SignalDisconnect(st); // signal client (error)
    }

  }

  // -----------------------------------------------------

  /*
   *  non-blocking poll on socket
   *  deals with all waiting packets
   *  (deprecated)
   */
  void Client::Poll() {
    RecvFromServer();
  }

  void Client::socket_cb(int fd, SocketEvent::Mode m) {
    if ( fd == m_serverSocket.getSocketHandle() ) {
      RecvFromServer();
    } else if ( fd == m_listenServer.getSocketHandle() ) {
      TCPSocket *sock = m_listenServer.Accept();
      DirectClient *dc = new DirectClient(sock, m_uin, 0, m_listenServer.getPort(), &m_translator);
      m_fdmap[ sock->getSocketHandle() ] = dc;
      dc->logger.connect( slot(this, &Client::SignalLog_cb) );
      dc->messaged.connect( slot(this, &Client::SignalMessageEvent_cb) );
      SignalAddSocket( sock->getSocketHandle(), SocketEvent::READ );
    } else {
      if (m_fdmap.count(fd) > 0) {
	DirectClient *dc = m_fdmap[fd];
	try {
	  dc->Recv();
	} catch(UINConfirmationException e) {
	  // thrown by DirectClient indicating
	  // we should confirm the UIN it has
	  if ( m_contact_list.exists( dc->getUIN() ) ) {
	    Contact &c = m_contact_list[ dc->getUIN() ];
	    if ( (c.getExtIP() == m_ext_ip && c.getLanIP() == dc->getIP() )
		 /* They are behind the same masquerading box,
		  * and the Lan IP matches
		  */
		 || c.getExtIP() == dc->getIP()) {
	      dc->setContact( &c );
	    } else {
	      // spoofing attempt most likely
	      ostringstream ostr;
	      ostr << "Refusing direct connection from someone that claims to be UIN "
		   << dc->getUIN() << " since IPs don't match with what server has told us." << endl;
	      SignalLog(LogEvent::WARN, ostr.str() );
	      DisconnectDirectConn( fd );
	    }

	  } else {
	    // add to contact list and wait for an online alert
	    // then handle the confirmation - need timeouts really!
	    // note to self: todo!
	    cout << "Lookup contact" << endl;
	  }
	} catch(DisconnectedException e) {
	  // tear down connection
	  DisconnectDirectConn( fd );
	}
      } else {
	cout << "Problem: unassociated socket" << endl;
      }
    }
  }

  int Client::Connect() {
    if (m_state == NOT_CONNECTED)
      ConnectAuthorizer(AUTH_AWAITING_CONN_ACK);
    return getSocketHandle();
  }
  int Client::RegisterUIN() {
    if (m_state == NOT_CONNECTED)
      ConnectAuthorizer(UIN_AWAITING_CONN_ACK);
    return getSocketHandle();
  }

  bool Client::isConnected() {
    return (m_state == BOS_LOGGED_IN);
  }

  void Client::HandleUserInfoSNAC(UserInfoSNAC *snac) {
    // this should only be personal info
    const UserInfoBlock &ub = snac->getUserInfo();
    if (ub.getUIN() == m_uin) {
      // currently only interested in our external IP
      // - we might be behind NAT
      if (ub.getExtIP() != 0) m_ext_ip = ub.getExtIP();
    }
  }

  void Client::SendEvent(MessageEvent *ev) {
    Buffer b(&m_translator);
    unsigned int d;
    d = FLAPHeader(b,0x02);

    Contact *c = ev->getContact();
    if (ev->getType() == MessageEvent::Normal) {
      NormalMessageEvent *nv = static_cast<NormalMessageEvent*>(ev);
      NormalICQSubType nist(nv->getMessage(), c->getUIN(), c->acceptAdvancedMsgs());

      MsgSendSNAC msnac(&nist);
      if (c->acceptAdvancedMsgs()) {
	msnac.setAdvanced(true);
	msnac.setSeqNum( c->nextSeqNum() );
      }
      b << msnac;

    } else if (ev->getType() == MessageEvent::URL) {
      URLMessageEvent *uv = static_cast<URLMessageEvent*>(ev);
      URLICQSubType uist(uv->getMessage(), uv->getURL(), m_uin, c->getUIN(), c->acceptAdvancedMsgs());

      MsgSendSNAC msnac(&uist);
      if (c->acceptAdvancedMsgs()) {
	msnac.setAdvanced(true);
	msnac.setSeqNum( c->nextSeqNum() );
      }
      b << msnac;

    } else if (ev->getType() == MessageEvent::SMS) {
      SMSMessageEvent *sv = static_cast<SMSMessageEvent*>(ev);
      SrvSendSNAC ssnac(sv->getMessage(), c->getMobileNo(), m_uin, "", sv->getRcpt());
      b << ssnac;
    }
    FLAPFooter(b,d);
    Send(b);
  }
  
  void Client::PingServer() {
    
    Buffer b(&m_translator);
    unsigned int d;
    d = FLAPHeader(b,0x05);
    FLAPFooter(b,d);
    Send(b);
  }

  void Client::setStatus(const Status st) {
    m_status = st;
    if (m_state == BOS_LOGGED_IN) {
      Buffer b(&m_translator);
      unsigned int d;
      
      d = FLAPHeader(b,0x02);
      SetStatusSNAC sss(MapStatusToICQStatus(m_status, m_invisible));
      b << sss;
      FLAPFooter(b,d);
      
      Send(b);
    }
  }

  Status Client::getStatus() {
    return m_status;
  }

  void Client::addContact(Contact& c) {

    if (!m_contact_list.exists(c.getUIN())) {

      m_contact_list.add(c);
      SignalUserAdded(&c);

      if (c.isICQContact() && m_state == BOS_LOGGED_IN) {
	Buffer b(&m_translator);
	unsigned int d;
	d = FLAPHeader(b,0x02);
	AddBuddySNAC abs(c);
	b << abs;
	FLAPFooter(b,d);

	Send(b);

	// fetch simple userinfo from server
	fetchSimpleContactInfo(&c);
      }
    }

  }

  void Client::removeContact(const unsigned int uin) {
    if (m_contact_list.exists(uin)) {
      SignalUserRemoved(&(m_contact_list[uin]));
      if (m_contact_list[uin].isICQContact() && m_state == BOS_LOGGED_IN) {
	Buffer b(&m_translator);
	unsigned int d;
	d = FLAPHeader(b,0x02);
	RemoveBuddySNAC rbs(Contact::UINtoString(uin));
	b << rbs;
	FLAPFooter(b,d);
	
	Send(b);
      }
      m_contact_list.remove(uin);
    }
  }
  
  Contact* Client::lookupICQ(unsigned int uin) {
    if (!m_contact_list.exists(uin)) {
      Contact c(uin);
      addContact(c);
    }
    return &(m_contact_list[uin]);
  }

  Contact* Client::lookupMobile(const string& m) {
    if (!m_contact_list.exists(m)) {
      Contact c(m,m);
      addContact(c);
    }
    return &(m_contact_list[m]);
  }

  

  void Client::SignalUserAdded(Contact *c) {
    UserAddedEvent ev(c);
    contactlist.emit(&ev);
  }

  void Client::SignalUserRemoved(Contact *c) {
    UserRemovedEvent ev(c);
    contactlist.emit(&ev);
  }

  void Client::SignalUserInfoChange(Contact *c) {
    UserInfoChangeEvent ev(c);
    contactlist.emit(&ev);
  }

  void Client::SignalMessageQueueChanged(Contact *c) {
    MessageQueueChangedEvent ev(c);
    contactlist.emit(&ev);
  }

  Contact* Client::getContact(const unsigned int uin) {
    if (m_contact_list.exists(uin)) {
      return &m_contact_list[uin];
    } else {
      return NULL;
    }
  }

  void Client::fetchSimpleContactInfo(Contact *c) {
    Buffer b(&m_translator);
    unsigned int d;

    if ( !c->isICQContact() ) return;

    d = FLAPHeader(b,0x02);
    SrvRequestDetailUserInfo ssnac( m_uin, m_buddy_uin = c->getUIN() );
    b << ssnac;
    FLAPFooter(b,d);

    Send(b);
  }

  void Client::fetchAwayMsg(Contact *c) {
    Buffer b(&m_translator);
    unsigned int d;

    if ( !c->isICQContact() ) return;
    if ( c->acceptAdvancedMsgs() ) {
      
      d = FLAPHeader(b,0x02);
      ReqAwayICQSubType ra( c->getStatus(), c->getUIN() );
      MsgSendSNAC msg( &ra, true );
      b << msg;
      FLAPFooter(b,d);
      
      Send(b);
    } else {
      // direct connection
    }
  }

  void Client::Disconnect() {
    if (m_state == NOT_CONNECTED) return;

    DisconnectInt();
    SignalDisconnect(DisconnectedEvent::REQUESTED);
    // signal we have disconnected - as requested
  }

  void Client::DisconnectInt() {
    if (m_state == NOT_CONNECTED) return;

    SignalLog(LogEvent::INFO, "Client disconnecting\n");

    if (m_state == AUTH_AWAITING_CONN_ACK || m_state == AUTH_AWAITING_AUTH_REPLY|| 
        m_state == UIN_AWAITING_CONN_ACK || m_state == UIN_AWAITING_UIN_REPLY) {
      DisconnectAuthorizer();
    } else {
      DisconnectBOS();
    }
  }

  unsigned short Client::MapStatusToICQStatus(Status st, bool inv) {
    unsigned short s;

    switch(st) {
    case STATUS_ONLINE:
      s = 0x0000;
      break;
    case STATUS_AWAY:
      s = 0x0001;
      break;
    case STATUS_NA:
      s = 0x0005;
      break;
    case STATUS_OCCUPIED:
      s = 0x0011;
      break;
    case STATUS_DND:
      s = 0x0013;
      break;
    case STATUS_FREEFORCHAT:
      s = 0x0020;
      break;
    }

    if (inv) s |= STATUS_FLAG_INVISIBLE;
    return s;
  }

  Status Client::MapICQStatusToStatus(unsigned short st) {
    if (st & STATUS_FLAG_DND) return STATUS_DND;
    else if (st & STATUS_FLAG_NA) return STATUS_NA;
    else if (st & STATUS_FLAG_OCCUPIED) return STATUS_OCCUPIED;
    else if (st & STATUS_FLAG_FREEFORCHAT) return STATUS_FREEFORCHAT;
    else if (st & STATUS_FLAG_AWAY) return STATUS_AWAY;
    else return STATUS_ONLINE;
  }

  bool Client::MapICQStatusToInvisible(unsigned short st) {
    return (st & STATUS_FLAG_INVISIBLE);
  }

  bool Client::setTranslationMap(const string& szMapFileName) { 
    try{
      m_translator.setTranslationMap(szMapFileName);
    } catch (TranslatorException e) {
      ostringstream ostr;
      ostr << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      return false; 
    }
    return true;
  }

}
