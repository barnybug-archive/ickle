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

namespace ICQ2000 {

  Client::Client() {
    Init();
  }

  Client::Client(const unsigned int uin, const string& password) : m_uin(uin), m_password(password) {
    Init();
  }

  Client::~Client() {
    if (m_cookie_data)
      delete [] m_cookie_data;
  }

  void Client::Init() {
    m_authorizerHostname = "login.icq.com";
    m_authorizerPort = 5190;

    m_state = NOT_CONNECTED;

    m_cookie_data = NULL;
    m_cookie_length = 0;

    m_status = STATUS_ONLINE;
  }

  unsigned short Client::NextSeqNum() {
    return m_client_seq_num++;
  }

  void Client::ConnectAuthorizer() {
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

    m_state = AUTH_AWAITING_CONN_ACK;
  }

  void Client::DisconnectAuthorizer() {
    m_serverSocket.Disconnect();
    m_state = NOT_CONNECTED;
  }

  void Client::ConnectBOS() {
    try {
      m_serverSocket.setRemoteHost(m_bosHostname.c_str());
      m_serverSocket.setRemotePort(m_bosPort);
      m_serverSocket.Connect();
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
    m_serverSocket.Disconnect();
    m_state = NOT_CONNECTED;
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

  void Client::SignalMessage(MessageSNAC *snac) {
    Contact *contact;
    MessageEvent *e;
    ICQSubType *st = snac->getICQSubType();

    if (st->getType() == MSG_Type_Normal) {
      NormalICQSubType *nst = static_cast<NormalICQSubType*>(st);
      
      UserInfoBlock userinfo = snac->getUserInfo();
      unsigned int uin = userinfo.getUIN();
      contact = lookupICQ(uin);
      e = new NormalMessageEvent(contact,
				 nst->getMessage());

    } else if (st->getType() == MSG_Type_URL) {
      URLICQSubType *ust = static_cast<URLICQSubType*>(st);
      
      UserInfoBlock userinfo = snac->getUserInfo();
      unsigned int uin = userinfo.getUIN();
      contact = lookupICQ(uin);
      e = new URLMessageEvent(contact,
			      ust->getMessage(),
			      ust->getURL());

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

    contact->addPendingMessage(e);

    if (messaged.emit(e)) {
      // true indicates the message was handled and
      // should be removed from the queue
      contact->erasePendingMessage(e);
    }

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
				   snac->getTime());
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
       * todo - need to do request ids
      if (snac->deliverable()) {
	SMSResponseEvent sev(snac->getSource(), snac->getNetwork());
	messaged.emit(&sev);
      } else {
	SMSResponseEvent sev(snac->getSource(), snac->getErrorId(), snac->getErrorParam());
	messaged.emit(&sev);
      }
      */

    } else if (snac->getType() == SrvResponseSNAC::SimpleUserInfo) {

      // update Contact
      if ( m_contact_list.exists( snac->getUIN() ) ) {
	Contact& c = m_contact_list[ snac->getUIN() ];
	c.setAlias( snac->getAlias() );
	UserInfoChangeEvent ev(&c);
	contactlist.emit(&ev);
      }

    }

  }

  void Client::SignalLog(LogEvent::LogType type, const string& msg) {
    LogEvent ev(type,msg);
    logger.emit(&ev);
  }

  void Client::SignalUserOnline(BuddyOnlineSNAC *snac) {
    const UserInfoBlock& userinfo = snac->getUserInfo();
    if (m_contact_list.exists(userinfo.getUIN())) {
      Contact& c = m_contact_list[userinfo.getUIN()];
      c.setStatus(MapICQStatusToStatus(userinfo.getStatus()));
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
    Buffer b;
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
    Send(b);
  }
    
  void Client::SendCookie() {
    Buffer b;
    unsigned int d = FLAPHeader(b,0x01);

    b << (unsigned int)0x00000001;

    b << CookieTLV(m_cookie_data, m_cookie_length);

    FLAPFooter(b,d);
    Send(b);
  }
    
  void Client::SendCapabilities() {
    Buffer b;
    unsigned int d = FLAPHeader(b,0x02);
    CapabilitiesSNAC cs;
    b << cs;
    FLAPFooter(b,d);
    Send(b);
  }

  void Client::SendLogin() {
    Buffer b;
    unsigned int d;

    if (!m_contact_list.empty()) {
      d = FLAPHeader(b,0x02);
      AddBuddySNAC abs(m_contact_list);
      b << abs;
      FLAPFooter(b,d);
    }

    d = FLAPHeader(b,0x02);
    SetStatusSNAC sss(MapStatusToICQStatus(m_status));
    b << sss;
    FLAPFooter(b,d);

    d = FLAPHeader(b,0x02);
    SetIdleSNAC sis;
    b << sis;
    FLAPFooter(b,d);

    d = FLAPHeader(b,0x02);
    ClientReadySNAC crs;
    b << crs;
    FLAPFooter(b,d);

    Send(b);
  }

  void Client::SendOfflineMessagesRequest() {
    Buffer b;
    unsigned int d;

    d = FLAPHeader(b,0x02);
    SrvRequestOfflineSNAC ssnac(m_uin);
    b << ssnac;
    FLAPFooter(b,d);

    Send(b);
  }


  void Client::SendOfflineMessagesACK() {
    Buffer b;
    unsigned int d;

    d = FLAPHeader(b,0x02);
    SrvAckOfflineSNAC ssnac(m_uin);
    b << ssnac;
    FLAPFooter(b,d);

    Send(b);
  }

  void Client::Send(Buffer& b) {
    try {
      ostringstream ostr;
      ostr << "Sending packet" << endl << b << endl;
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

  bool Client::Recv() {
    if (!m_serverSocket.connected()) return false;

    try {
      Buffer b;
      if (m_serverSocket.RecvNonBlocking(m_recv)) {
	ostringstream ostr;
	ostr << "Received packet" << endl << m_recv << endl;
	SignalLog(LogEvent::PACKET, ostr.str());
	Parse();
	return true;
      } else {
	return false;
      }
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed on recv: " << e.what() << endl;
      SignalLog(LogEvent::WARN, ostr.str());
      Disconnect();
    }

    return false;

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
      Buffer sb;
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

    if (b.remains() == 4 && m_state == AUTH_AWAITING_CONN_ACK) {

      // Connection Acknowledge - first packet from server on connection
      unsigned int unknown;
      b >> unknown; // always 0x0001
      SignalLog(LogEvent::INFO, "Connection Acknowledge from server, sending Authorisation request\n");

      SendAuthReq();
      m_state = AUTH_AWAITING_AUTH_REPLY;

    } else if (b.remains() == 4 && m_state == BOS_AWAITING_CONN_ACK) {

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
      snac = InSNAC::ParseSNAC(b);
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
	SendCapabilities();
	break;
      case SNAC_GEN_CapAck:
	SendLogin();
	break;
      case SNAC_GEN_MOTD:
	SignalConnect();
	SendOfflineMessagesRequest();
	/* take the MOTD as the sign we
	 * are online proper
	 */
	break;
      }
      break;

    case SNAC_FAM_BUD:
      switch(snac->Subtype()) {
      case SNAC_BUD_Online:
	SignalUserOnline((BuddyOnlineSNAC*)snac);
	break;
      case SNAC_BUD_Offline:
	SignalUserOffline((BuddyOfflineSNAC*)snac);
      }
      break;

    case SNAC_FAM_MSG:
      switch(snac->Subtype()) {
      case SNAC_MSG_Message:
	SignalMessage((MessageSNAC*)snac);
	break;
      }
      break;

    case SNAC_FAM_SRV:
      switch(snac->Subtype()) {
      case SNAC_SRV_Response:
	SignalSrvResponse((SrvResponseSNAC*)snac);
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
    if (m_state == AUTH_AWAITING_AUTH_REPLY) {
      // An Authorisation Reply / Error
      TLVList tlvlist;
      tlvlist.Parse(b, TLV_ParseMode_Channel04, -1);

      if (tlvlist.exists(TLV_Cookie) && tlvlist.exists(TLV_Redirect)) {

	RedirectTLV *r = (RedirectTLV*)tlvlist[TLV_Redirect];
	ostringstream ostr;
	ostr << "Redirected to: " << r->getHost() << " port: " << dec << r->getPort() << endl;
	SignalLog(LogEvent::INFO, ostr.str());

	m_bosHostname = r->getHost();
	m_bosPort = r->getPort();

	// Got our cookie - yum yum :-)
	CookieTLV *t = (CookieTLV*)tlvlist[TLV_Cookie];
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
	  ErrorCodeTLV *t = (ErrorCodeTLV*)tlvlist[TLV_ErrorCode];
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
	} else {
	  SignalLog(LogEvent::WARN, "Error logging in, no error code given(?!)\n");
	  st = DisconnectedEvent::FAILED_UNKNOWN;
	}
	DisconnectAuthorizer();
	SignalDisconnect(st); // signal client (error)
      }

    } else {
      SignalLog(LogEvent::WARN, "Unexpected packet received on channel 0x04\n");
    }

  }

  // -----------------------------------------------------

  /*
   *  non-blocking poll on socket
   *  deals with all waiting packets
   */
  void Client::Poll() {
    while (Client::Recv()) { };
  }

  int Client::Connect() {
    if (m_state == NOT_CONNECTED)
      ConnectAuthorizer();
    return getSocketHandle();
  }

  bool Client::isConnected() {
    return (m_state == BOS_LOGGED_IN);
  }

  void Client::SendEvent(MessageEvent *ev) {
    Buffer b;
    unsigned int d;
    d = FLAPHeader(b,0x02);

    Contact *c = ev->getContact();
    if (ev->getType() == MessageEvent::Normal) {
      NormalMessageEvent *nv = static_cast<NormalMessageEvent*>(ev);
      NormalICQSubType nist(nv->getMessage(), c->getUIN());
      MsgSendSNAC msnac(&nist);
      b << msnac;
    } else if (ev->getType() == MessageEvent::URL) {
      URLMessageEvent *uv = static_cast<URLMessageEvent*>(ev);
      URLICQSubType uist(uv->getMessage(), uv->getURL(), m_uin, c->getUIN());
      MsgSendSNAC msnac(&uist);
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
    
    Buffer b;
    unsigned int d;
    d = FLAPHeader(b,0x05);
    FLAPFooter(b,d);
    Send(b);
  }

  void Client::setStatus(const Status st) {
    m_status = st;
    if (m_state == BOS_LOGGED_IN) {
      Buffer b;
      unsigned int d;
      
      d = FLAPHeader(b,0x02);
      SetStatusSNAC sss(MapStatusToICQStatus(m_status));
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
	Buffer b;
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
	Buffer b;
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
    Buffer b;
    unsigned int d;

    if ( !c->isICQContact() ) return;

    d = FLAPHeader(b,0x02);
    SrvRequestSimpleUserInfo ssnac( m_uin, c->getUIN() );
    b << ssnac;
    FLAPFooter(b,d);

    Send(b);
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

    if (m_state == AUTH_AWAITING_CONN_ACK || m_state == AUTH_AWAITING_AUTH_REPLY) {
      DisconnectAuthorizer();
    } else {
      DisconnectBOS();
    }
  }

  unsigned short Client::MapStatusToICQStatus(Status st) {
    switch(st) {
    case STATUS_ONLINE:
      return 0x0000;
    case STATUS_AWAY:
      return 0x0001;
    case STATUS_NA:
      return 0x0005;
    case STATUS_OCCUPIED:
      return 0x0011;
    case STATUS_DND:
      return 0x0013;
    case STATUS_FREEFORCHAT:
      return 0x0020;
      //    case STATUS_INVISIBLE:
      //      return 0x0100;
    }
  }

  Status Client::MapICQStatusToStatus(unsigned short st) {
    if (st & STATUS_FLAG_DND) return STATUS_DND;
    else if (st & STATUS_FLAG_NA) return STATUS_NA;
    else if (st & STATUS_FLAG_OCCUPIED) return STATUS_OCCUPIED;
    else if (st & STATUS_FLAG_FREEFORCHAT) return STATUS_FREEFORCHAT;
    else if (st & STATUS_FLAG_AWAY) return STATUS_AWAY;
    else return STATUS_ONLINE;
  }

}
