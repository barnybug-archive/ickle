/*
 * SNACs
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

#include "SNAC.h"

namespace ICQ2000 {

  // ------------------ Abstract SNACs ---------------


  InSNAC* InSNAC::ParseSNAC(Buffer& b) {
    unsigned short family, subtype;
    b >> family
      >> subtype;

    InSNAC *snac = NULL;

    switch(family) {

    case SNAC_FAM_GEN:
      switch(subtype) {
      case SNAC_GEN_ServerReady:
	snac = new ServerReadySNAC();
	break;
      case SNAC_GEN_CapAck:
	snac = new CapAckSNAC();
	break;
      case SNAC_GEN_UserInfo:
	snac = new UserInfoSNAC();
	break;
      case SNAC_GEN_MOTD:
	snac = new MOTDSNAC();
	break;
      }
      break;

    case SNAC_FAM_BUD:
      switch(subtype) {
      case SNAC_BUD_Online:
	snac = new BuddyOnlineSNAC();
	break;
      case SNAC_BUD_Offline:
	snac = new BuddyOfflineSNAC();
	break;
      }
      break;

    case SNAC_FAM_MSG:
      switch(subtype) {
      case SNAC_MSG_Message:
	snac = new MessageSNAC();
	break;
      case SNAC_MSG_SentOffline:
	snac = new MessageSentOfflineSNAC();
	break;
      }
      break;

    case SNAC_FAM_SRV:
      switch(subtype) {
      case SNAC_SRV_Response:
	snac = new SrvResponseSNAC();
	break;
      }
      break;
    case SNAC_FAM_UIN:
      switch(subtype) {
      case SNAC_UIN_Response:
	snac = new UINResponseSNAC();
	break;
      }
         
      break;

    }
    
    if (snac == NULL) {
      // unrecognised SNAC
      // parse as a RawSNAC
      snac = new RawSNAC(family, subtype);
    }

    snac->Parse(b);

    return snac;

  }

  void InSNAC::Parse(Buffer& b) {
    b >> m_flags
      >> m_requestID;
    ParseBody(b);
  }

  unsigned short InSNAC::Flags() const {
    return m_flags;
  }

  unsigned int InSNAC::RequestID() const {
    return m_requestID;
  }

  void OutSNAC::Output(Buffer& b) const {
    OutputHeader(b);
    OutputBody(b);
  }

  void OutSNAC::OutputHeader(Buffer& b) const {
    b << Family();
    b << Subtype();
    b << Flags();
    b << RequestID();
  }

  // --------------- Raw SNAC ---------------------------

  RawSNAC::RawSNAC(unsigned short f, unsigned short t)
    : m_family(f), m_subtype(t) { }

  void RawSNAC::ParseBody(Buffer& b) {
    unsigned char c;
    while(b.beforeEnd()) {
      b >> c;
      m_data << c;
    }
  }

  // --------------- Generic (Family 0x0001) ------------

  void ServerReadySNAC::ParseBody(Buffer& b) {
    /* The body of the server ready SNAC seems
     * to be a list of the SNAC families the server
     * will accept - the client is then expected
     * to send back a list of those it wants
     * - basically ignore this for the moment :-)
     */
    unsigned short cap;
    while(b.beforeEnd())
      b >> cap;

  }

  void CapabilitiesSNAC::OutputBody(Buffer& b) const {
    /* doesn't seem any need currently to do more
     * than copy the official client
     */
    unsigned short v1 = 0x0001, v3 = 0x0003;
    b << SNAC_FAM_GEN << v3
      << SNAC_FAM_LOC << v1
      << SNAC_FAM_BUD << v1
      << SNAC_FAM_SRV << v1
      << SNAC_FAM_MSG << v1
      << SNAC_FAM_INV << v1
      << SNAC_FAM_BOS << v1
      << SNAC_FAM_LUP << v1;
  }

  void CapAckSNAC::ParseBody(Buffer& b) {
    /* server sends back the list from ServerReady again
     * but with versions of families included
     * - again ignore for the moment
     */
    unsigned short cap, ver;
    while(b.beforeEnd()) {
      b >> cap
	>> ver;
    }

  }

  SetStatusSNAC::SetStatusSNAC(unsigned short status)
    : m_status(status) { }

  void SetStatusSNAC::OutputBody(Buffer& b) const {
    StatusTLV stlv(ALLOWDIRECT_EVERYONE, WEBAWARE_NORMAL, m_status);
    b << stlv;
    //    LANDetailsTLV ltlv;
    //    b << ltlv;
  }

  void SetIdleSNAC::OutputBody(Buffer& b) const {
    /* don't know what this value means exactly */
    b << (unsigned int)0x00000000;
  }

  void ClientReadySNAC::OutputBody(Buffer& b) const {
    /* related to capabilities again
     * - figure this out sometime
     */
    b << 0x00010003
      << 0x0110028a
      << 0x00020001
      << 0x0101028a
      << 0x00030001
      << 0x0110028a
      << 0x00150001
      << 0x0110028a
      << 0x00040001
      << 0x0110028a
      << 0x00060001
      << 0x0110028a
      << 0x00090001
      << 0x0110028a
      << 0x000a0001
      << 0x0110028a;
  }

  void UserInfoSNAC::ParseBody(Buffer& b) {
    m_userinfo.Parse(b);
  }

  void MOTDSNAC::ParseBody(Buffer& b) {
    b >> m_status;

    /* as far as I know only one TLV follows,
     * but we'll treat it as a list
     */
    TLVList tlvlist;
    tlvlist.Parse(b, TLV_ParseMode_Channel02, (short unsigned int)-1);
    if (tlvlist.exists(TLV_WebAddress)) {
      WebAddressTLV *t = (WebAddressTLV*)tlvlist[TLV_WebAddress];
      m_url = t->Value();
    }
  }

  // --------------- Buddy List (Family 0x0003) SNACs --------------

  AddBuddySNAC::AddBuddySNAC() { }

  AddBuddySNAC::AddBuddySNAC(const ContactList& l)
    : m_buddy_list() { 
    ContactList::const_iterator curr = l.begin();
    while (curr != l.end()) {
      if ((*curr).isICQContact()) m_buddy_list.push_back((*curr).getStringUIN());
      ++curr;
    }
    
  }

  AddBuddySNAC::AddBuddySNAC(const Contact& c)
    : m_buddy_list(1, c.getStringUIN()) { }

  void AddBuddySNAC::addBuddy(const Contact& c) {
    m_buddy_list.push_back(c.getStringUIN());
  }

  void AddBuddySNAC::OutputBody(Buffer& b) const {
    list<string>::const_iterator curr = m_buddy_list.begin();
    while (curr != m_buddy_list.end()) {
      b << (unsigned char)(*curr).size();
      b.Pack(*curr);
      curr++;
    }
  }

  RemoveBuddySNAC::RemoveBuddySNAC() { }

  RemoveBuddySNAC::RemoveBuddySNAC(const ContactList& l)
    : m_buddy_list() { 
    ContactList::const_iterator curr = l.begin();
    while (curr != l.end()) {
      if ((*curr).isICQContact()) m_buddy_list.push_back((*curr).getStringUIN());
      ++curr;
    }
    
  }

  RemoveBuddySNAC::RemoveBuddySNAC(const string& s)
    : m_buddy_list(1, s) { }

  void RemoveBuddySNAC::removeBuddy(const Contact& c) {
    m_buddy_list.push_back(c.getStringUIN());
  }

  void RemoveBuddySNAC::OutputBody(Buffer& b) const {
    list<string>::const_iterator curr = m_buddy_list.begin();
    while (curr != m_buddy_list.end()) {
      b << (unsigned char)(*curr).size();
      b.Pack(*curr);
      curr++;
    }
  }

  BuddyOnlineSNAC::BuddyOnlineSNAC() { }

  void BuddyOnlineSNAC::ParseBody(Buffer& b) {
    /* All BuddyOnline consists of is the user info
     * block TLVs
     */
    m_userinfo.Parse(b);
  }

  BuddyOfflineSNAC::BuddyOfflineSNAC() { }

  void BuddyOfflineSNAC::ParseBody(Buffer& b) {
    m_userinfo.Parse(b);
  }

  // --------------- Message (Family 0x0004) SNACs -----------------

  MsgSendSNAC::MsgSendSNAC(ICQSubType *icqsubtype)
    : m_icqsubtype(icqsubtype) { } 

  void MsgSendSNAC::OutputBody(Buffer& b) const {

    /*
     * ICBM Cookie - (we ignore it)
     */
    b << (unsigned int)0x00000000
      << (unsigned int)0x00000000;
    
    /* There is no consistency in the protocol here,
     * Messages are sent on channel 1
     * Everything else on channel 4
     */
    if (m_icqsubtype->getType() == MSG_Type_Normal) {
      NormalICQSubType *nst = static_cast<NormalICQSubType*>(m_icqsubtype);

      b << (unsigned short)0x0001;

      // Destination UIN (screenname)
      string sn = Contact::UINtoString(nst->getDestination());
      b << (unsigned char)sn.size();
      b.Pack(sn);
      
      string m_text = nst->getMessage();
      ICQSubType::LFtoCRLF(m_text);
      unsigned short text_size = m_text.size();

      /*
       * Message Block TLV
       * - contains two TLVs
       * 0x0501 - don't know what this is
       * 0x0101 - the message
       */
      b << (unsigned short)0x0002
	<< (unsigned short)(13+text_size);
      
      // TLV 0x0501
      b << (unsigned short)0x0501
	<< (unsigned short)0x0001
	<< (unsigned char) 0x01;
      
      // TLV 0x0101
      b << (unsigned short)0x0101
	<< (unsigned short)(4+text_size);
      
      // flags
      b << (unsigned short)0x0000
	<< (unsigned short)0x0000;
      
      b.Pack(m_text);
      
    } else if (m_icqsubtype->getType() == MSG_Type_URL) {
      URLICQSubType *ust = static_cast<URLICQSubType*>(m_icqsubtype);
      
      b << (unsigned short)0x0004;

      // Destination UIN (screenname)
      string sn = Contact::UINtoString(ust->getDestination());
      b << (unsigned char)sn.size();
      b.Pack(sn);

      string m_text, m_url;
      m_text = ust->getMessage();
      m_url = ust->getURL();
      ICQSubType::LFtoCRLF(m_text);
      ICQSubType::LFtoCRLF(m_url);
      unsigned short total_size = m_text.size() + 2 + m_url.size();

      /* Data Block TLV
       */
      b << (unsigned short)0x0005
	<< (unsigned short)(8+total_size);

      b.setEndianness(Buffer::LITTLE);
      b << (unsigned int)ust->getSource()
	<< (unsigned short)MSG_Type_URL // ICQ Subtype
	<< (unsigned short)total_size;
      b.Pack(m_text);
      b << (unsigned char)0xfe; // separator
      b.Pack(m_url);
      b << (unsigned char)0x00; // null terminated
    }

    /* Another TLV - dunno what it means
     * - it doesn't seem to matter if I take this out
     */
    b.setEndianness(Buffer::BIG);
    b << (unsigned short)0x0006
      << (unsigned short)0x0000;

  }

  MessageSNAC::MessageSNAC() : m_icqsubtype(NULL) { }

  MessageSNAC::~MessageSNAC() {
    if (m_icqsubtype != NULL) delete m_icqsubtype;
  }

  void MessageSNAC::ParseBody(Buffer& b) {
    /*
     * ICBM Cookie - (we ignore it)
     */
    unsigned char cookie[8];
    for (int i = 0; i < 8; i++)
      b >> cookie[i];

    /*
     * Channel 0x0001 = Normal message
     * Channel 0x0002 = Rendezvous (client-client stuff)
     * Channel 0x0004 = ICQ specific features (URLs, Added to Contact List, SMS, etc.)
     */
    unsigned short channel;
    b >> channel;
    if (channel != 0x0001 && channel != 0x0004)
      throw ParseException("Message SNAC 0x0004 0x0007 received on unknown channel");

    /*
     * the UserInfo block comes next
     * this is a screenname, then some tlvs
     */
    m_userinfo.Parse(b);

    /*
     * the Message block comes now
     * this is one (maybe more TLVs) with the message garbled
     * up inside
     */
    TLVList tlvlist;
    tlvlist.Parse(b, TLV_ParseMode_MessageBlock, (short unsigned int)-1);
    
    if (channel == 0x0001) {
      // Normal message
      if (tlvlist.exists(TLV_MessageData)) {

	MessageDataTLV *t = (MessageDataTLV*)tlvlist[TLV_MessageData];
	// coerce this into the NormalICQSubType
	NormalICQSubType *nst = new NormalICQSubType(false);
	nst->setMessage( t->getMessage() );
	m_icqsubtype = nst;

      } else {
	throw ParseException("No message data in SNAC");
      }

    } else if (channel == 0x0004) {

      /* ICQ hacked in messages
       * - SMS message
       * - URLs
       * - Added to contactlist
       * ..
       */

      if (tlvlist.exists(TLV_ICQData)) {
	ICQDataTLV *t = (ICQDataTLV*)tlvlist[TLV_ICQData];
	m_icqsubtype = t->grabICQSubType();
      } else {
	throw ParseException("No ICQ data TLV in SNAC 0x0004 0x0007 on channel 4");
      }

    } else {

      ostringstream ostr;
      ostr << "Message SNAC on unsupported channel: 0x" << hex << channel;
      throw ParseException(ostr.str());

    }
    
  }

  void MessageSentOfflineSNAC::ParseBody(Buffer& b) {
    b.advance(10);

    unsigned char len;
    string sn;
    b >> len;
    b.Unpack(sn, len);
    m_uin = Contact::StringtoUIN(sn);
  }


  // --------------------- Server (Family 0x0015) SNACs ---------

  SrvSendSNAC::SrvSendSNAC(const string& text, const string& destination,
			   unsigned int senders_UIN, const string& senders_name, bool delrpt)
    : m_text(text), m_destination(destination), m_senders_UIN(senders_UIN),
      m_senders_name(senders_name), m_delivery_receipt(delrpt) { } 

  void SrvSendSNAC::OutputBody(Buffer& b) const {

    /*
     * Sending SMS messages
     * This is the biggest hodge-podge of a mess you
     * could imagine in a protocol, a mix of Big and Little Endian,
     * AIM TLVs and ICQ binary data and add in some XML
     * to top it all off. :-)
     */
    
    XmlBranch xmltree("icq_sms_message");
    xmltree.pushnode(new XmlLeaf("destination",m_destination));
    xmltree.pushnode(new XmlLeaf("text",m_text));
    xmltree.pushnode(new XmlLeaf("codepage","1252"));
    xmltree.pushnode(new XmlLeaf("senders_UIN",Contact::UINtoString(m_senders_UIN)));
    xmltree.pushnode(new XmlLeaf("senders_name",m_senders_name));
    xmltree.pushnode(new XmlLeaf("delivery_receipt",(m_delivery_receipt ? "Yes" : "No")));

    /* Time string, format: Wkd, DD Mnm YYYY HH:MM:SS TMZ */
    char timestr[30];
    time_t t;
    struct tm *tm;
    time(&t);
    tm = gmtime(&t);
    strftime(timestr, 30, "%a, %d %b %Y %T %Z", tm);
    xmltree.pushnode(new XmlLeaf("time",string(timestr)));

    string xmlstr = xmltree.toString(0);

    // this is a TLV header
    b << (unsigned short)0x0001;
    b << (unsigned short)(xmlstr.size()+37);

    b.setEndianness(Buffer::LITTLE);
    b << (unsigned short)(xmlstr.size()+35);
    b << m_senders_UIN;

    // think this is the message type
    b << (unsigned short)2000;

    // think this is a request id of sorts
    b << (unsigned short)0x0001;

    b.setEndianness(Buffer::BIG);

    // SMS send subtype
    b << (unsigned short)0x8214;

    // not sure about what this means
    b << (unsigned short)0x0001;
    b << (unsigned short)0x0016;
    for(int a = 0; a < 16; a++)
      b << (unsigned char)0x00;

    // not sure whether this is really an int
    b << (unsigned int)(xmlstr.size()+1);

    b.Pack(xmlstr);
    b << (unsigned char)0x00; // NULL terminated
  }

  SrvRequestOfflineSNAC::SrvRequestOfflineSNAC(unsigned int uin)
    : m_uin(uin) { }

  void SrvRequestOfflineSNAC::OutputBody(Buffer& b) const {
    // this is a TLV header
    b << (unsigned short)0x0001
      << (unsigned short)0x000a;

    b.setEndianness(Buffer::LITTLE);
    b << (unsigned short)0x0008;
    b << m_uin;

    // message type
    b << (unsigned short)60;
    // a request id
    b << (unsigned short)0x0000;

  }

  SrvAckOfflineSNAC::SrvAckOfflineSNAC(unsigned int uin)
    : m_uin(uin) { }

  void SrvAckOfflineSNAC::OutputBody(Buffer& b) const {
    // this is a TLV header
    b << (unsigned short)0x0001
      << (unsigned short)0x000a;

    b.setEndianness(Buffer::LITTLE);
    b << (unsigned short)0x0008;
    b << m_uin;

    // message type
    b << (unsigned short)62;
    // a request id
    b << (unsigned short)0x0000;

  }

  SrvRequestSimpleUserInfo::SrvRequestSimpleUserInfo(unsigned int my_uin, unsigned int user_uin)
    : m_my_uin(my_uin), m_user_uin(user_uin) { }

  void SrvRequestSimpleUserInfo::OutputBody(Buffer& b) const {
    b << (unsigned short)0x0001
      << (unsigned short)0x0010;

    b.setEndianness(Buffer::LITTLE);
    b << (unsigned short)0x000e;
    b << m_my_uin;

    b << (unsigned short)2000
      << (unsigned short)0x0000
      << (unsigned short)1311
      << m_user_uin;
    
  }

  SrvResponseSNAC::SrvResponseSNAC() : m_icqsubtype(NULL) { }

  SrvResponseSNAC::~SrvResponseSNAC() {
    if (m_icqsubtype != NULL) delete m_icqsubtype;
  }

  void SrvResponseSNAC::ParseBody(Buffer& b) {

    /* It is worth making the distinction between
     * sms responses and sms delivery responses
     * - an sms response is sent on this channel always
     *   after an sms is sent
     * - an sms delivery response is sent from the mobile
     *   if requested and arrives as SNAC 0x0004 0x0007
     */

    // a TLV header
    unsigned short type, length;
    b >> type;
    b >> length;
    
    b.setEndianness(Buffer::LITTLE);
    // the length again in little endian
    b >> length;

    unsigned int uin;
    b >> uin;

    /* Command type:
     * 65 (dec) = An Offline message
     * 66 (dec) = Offline Messages Finish
     * 2010 (dec) = SMS delivery response
     * others.. ??
     */
    unsigned short command_type;
    b >> command_type;

    unsigned short request_id;
    b >> request_id;

    if (command_type == 65) {
      ParseOfflineMessage(b);
    } else if (command_type == 66) {
      m_type = OfflineMessagesComplete;
      unsigned char waste_char;
      b >> waste_char;
    } else if (command_type == 2010) {
      ParseICQResponse(b);
    } else {
      throw ParseException("Unknown command type for Server Response SNAC");
    }

  }

  void SrvResponseSNAC::ParseOfflineMessage(Buffer& b) {
    b >> m_sender_UIN;
    unsigned short year;
    unsigned char month, day, hour, minute;
    b >> year
      >> month
      >> day
      >> hour
      >> minute;

    struct tm timetm;
    timetm.tm_sec = 0;
    timetm.tm_min = minute;
    timetm.tm_hour = hour;
    timetm.tm_mday = day;
    timetm.tm_mon = month-1;
    timetm.tm_year = year-1900;
    
    m_time = mktime(&timetm);

    m_type = OfflineMessage;
    m_icqsubtype = ICQSubType::ParseICQSubType(b);
    b.advance(2); // unknown
  }

  void SrvResponseSNAC::ParseICQResponse(Buffer& b) {

    /* Subtype
     * 1 = an error
     * 100 = sms response - problem 
     * 150 = sms response - ok ??
     * 410 = simple user info
     */
    unsigned short subtype;
    b >> subtype;

    if (subtype == 1)
      ParseSMSError(b);
    else if (subtype == 100 || subtype == 150)
      ParseSMSResponse(b);
    else if (subtype == 410)
      ParseSimpleUserInfo(b);
    else
      throw ParseException("Unknown ICQ subtype for Server response SNAC");

  }

  void SrvResponseSNAC::ParseSMSError(Buffer& b) {
    m_type = SMS_Error;
    // to do - maybe?
  }

  void SrvResponseSNAC::ParseSMSResponse(Buffer& b) {
    /* Not sure what the difference between 100 and 150 is
     * when successful it sends the erroneous 100 and then 150
     * otherwise only 150 I think
     */
    m_type = SMS_Response;

    /* Don't know what the next lot of data
       * means:
       * 0a 00 01 00 08 00 01
       */
    unsigned char waste_char;
    for (int a = 0; a < 7; a++)
      b >> waste_char;

    b.setEndianness(Buffer::BIG);
    string tag;
    b >> tag;

    string xmlstr;
    b >> xmlstr;

    string::iterator s = xmlstr.begin();
    auto_ptr<XmlNode> top(XmlNode::parse(s, xmlstr.end()));
    
    if (top.get() == NULL) throw ParseException("Couldn't parse xml data in Server Response SNAC");

    if (top->getTag() != "sms_response") throw ParseException("No <sms_response> tag found in xml data");
    XmlBranch *sms_response = dynamic_cast<XmlBranch*>(top.get());
    if (sms_response == NULL) throw ParseException("No tags found in xml data");

    XmlLeaf *source = sms_response->getLeaf("source");
    if (source != NULL) m_source = source->getValue();

    XmlLeaf *deliverable = sms_response->getLeaf("deliverable");
    m_deliverable = false;
    if (deliverable != NULL) {
      if (deliverable->getValue() == "Yes") m_deliverable = true;
      if (deliverable->getValue() == "SMTP")
	throw ParseException("SMS messages for your provider must be sent via an SMTP (email) proxy, "
			     "ickle doesn't support that yet, but may in the future.");
    }

    if (m_deliverable) {
      // -- deliverable = Yes --

      XmlLeaf *network = sms_response->getLeaf("network");
      if (network != NULL) m_network = network->getValue();

      XmlLeaf *message_id = sms_response->getLeaf("message_id");
      if (message_id != NULL) m_message_id = message_id->getValue();

      XmlLeaf *messages_left = sms_response->getLeaf("messages_left");
      if (messages_left != NULL) m_messages_left = messages_left->getValue();
      // always 0, unsurprisingly

    } else {
      // -- deliverable = No --

      // should be an <error> tag
      XmlBranch *error = sms_response->getBranch("error");
      if (error != NULL) {
	// should be an <id> tag
	XmlLeaf *error_id = error->getLeaf("id");
	if (error_id != NULL) {
	  // convert error id to a number
	  istringstream istr(error_id->getValue());
	  m_error_id = 0;
	  istr >> m_error_id;
	}

	// should also be a <params> branch
	XmlBranch *params = error->getBranch("params");
	if (params != NULL) {
	  // assume only one <param> tag
	  XmlLeaf *param = params->getLeaf("param");
	  if (param != NULL) m_error_param = param->getValue();
	}
      } // end <error> tag


    } // end deliverable = No


  }

  void SrvResponseSNAC::ParseSimpleUserInfo(Buffer& b) {
    m_type = SimpleUserInfo;

    unsigned char wb;
    b >> wb; // status code ?

    unsigned short ws;
    b >> ws; // unknown

    b >> m_uin;

    b.UnpackUint16StringNull(m_alias);
    b.UnpackUint16StringNull(m_first_name);
    b.UnpackUint16StringNull(m_last_name);
    b.UnpackUint16StringNull(m_email);

    // Auth required
    b >> wb;
    if (wb == 0) m_authreq = true;
    else m_authreq = false;

    // Status
    b >> m_status;

    b >> wb; // unknown

    unsigned int wi;
    b >> wi; // end marker ?

  }


  // -------------- New UIN (0x0017) Family -----------------------

  UINRequestSNAC::UINRequestSNAC(const string& p)
    : m_password(p) { }

  void UINRequestSNAC::OutputBody(Buffer& b) const{
    b<<(unsigned int)0x00010039;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x28000300;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x624e0000;
    b<<(unsigned int)0x624e0000;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x0;
    b.setEndianness(Buffer::LITTLE);
    b<<(unsigned short)m_password.length();
    b<<m_password.c_str();
    b.setEndianness(Buffer::BIG);
    b<<(unsigned int)0x624e0000;
    b<<(unsigned int)0x0000d601;
  }

  UINResponseSNAC::UINResponseSNAC() { }

  void UINResponseSNAC::ParseBody(Buffer& b){
    b.advance(46);
    b.setEndianness(Buffer::LITTLE);
    b >> m_uin;
    b.advance(10);
  }

  // -------------- Other Stuff -----------------------------------

  string UserInfoBlock::getScreenName() const { return m_screenname; }

  unsigned int UserInfoBlock::getUIN() const {
    return Contact::StringtoUIN(m_screenname);
  }
  
  unsigned int UserInfoBlock::getTimeOnline() const { return m_timeOnline; }
  unsigned int UserInfoBlock::getLanIP() const { return m_lan_ip; }
  unsigned int UserInfoBlock::getExtIP() const { return m_ext_ip; }
  unsigned short UserInfoBlock::getLanPort() const { return m_lan_port; }
  unsigned short UserInfoBlock::getExtPort() const { return m_ext_port; }
  unsigned short UserInfoBlock::getFirewall() const { return m_firewall; }
  unsigned char UserInfoBlock::getTCPVersion() const { return m_tcp_version; }
  unsigned short UserInfoBlock::getStatus() const { return m_status; }

  void UserInfoBlock::Parse(Buffer& b) {
    // (byte)length, string screenname
    unsigned char len;
    b >> len;
    b.Unpack(m_screenname, len);

    b >> m_warninglevel;
    unsigned short no_tlvs;
    b >> no_tlvs;
    
    TLVList tlvlist;
    tlvlist.Parse(b, TLV_ParseMode_Channel02, no_tlvs);

    m_userClass = 0;
    if (tlvlist.exists(TLV_UserClass)) {
      UserClassTLV *t = (UserClassTLV*)tlvlist[TLV_UserClass];
      m_userClass = t->Value();
    }

    m_status = 0;
    m_allowDirect = 0;
    m_webAware = 0;
    if (tlvlist.exists(TLV_Status)) {
      StatusTLV *t = (StatusTLV*)tlvlist[TLV_Status];
      m_allowDirect = t->getAllowDirect();
      m_webAware = t->getWebAware();
      m_status = t->getStatus();
    }

    m_timeOnline = 0;
    if (tlvlist.exists(TLV_TimeOnline)) {
      TimeOnlineTLV *t = (TimeOnlineTLV*)tlvlist[TLV_TimeOnline];
      m_timeOnline = t->Value();
    }

    m_signupDate = 0;
    if (tlvlist.exists(TLV_SignupDate)) {
      SignupDateTLV *t = (SignupDateTLV*)tlvlist[TLV_SignupDate];
      m_signupDate = t->Value();
    }

    m_signonDate = 0;
    if (tlvlist.exists(TLV_SignonDate)) {
      SignonDateTLV *t = (SignonDateTLV*)tlvlist[TLV_SignonDate];
      m_signonDate = t->Value();
    }

    m_lan_ip = 0;
    m_lan_port = 0;
    m_firewall = 0;
    m_tcp_version = 0;
    if (tlvlist.exists(TLV_LANDetails)) {
      LANDetailsTLV *t = (LANDetailsTLV*)tlvlist[TLV_LANDetails];
      m_lan_ip = t->getLanIP();
      m_lan_port = t->getLanPort();
      m_firewall = t->getFirewall();
      m_tcp_version = t->getTCPVersion();
    }

    m_ext_ip = 0;
    if (tlvlist.exists(TLV_IPAddress)) {
      IPAddressTLV *t = (IPAddressTLV*)tlvlist[TLV_IPAddress];
      m_ext_ip = t->Value();
    }

    m_ext_port = 0;
    if (tlvlist.exists(TLV_Port)) {
      PortTLV *t = (PortTLV*)tlvlist[TLV_Port];
      m_ext_port = t->Value();
    }

  }

}

Buffer& operator<<(Buffer& b, const ICQ2000::OutSNAC& snac) { snac.Output(b); return b; }
