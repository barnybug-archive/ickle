/*
 * TLVs
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

#include "TLV.h"

namespace ICQ2000 {

  static const unsigned char XORtable[] = { 0xf3, 0x26, 0x81, 0xc4,
					    0x39, 0x86, 0xdb, 0x92,
					    0x71, 0xa3, 0xb9, 0xe6,
					    0x53, 0x7a, 0x95, 0x7c };
  
  // ------------------ Generic TLV ---------------


  InTLV* InTLV::ParseTLV(Buffer& b, TLV_ParseMode parsemode) {
    unsigned short type;
    b >> type;

    InTLV *tlv = NULL;

    switch(parsemode) {

    // ----- CHANNEL 1 -----
    case TLV_ParseMode_Channel01:
      switch(type) {
      case TLV_Screenname:
	tlv = new ScreenNameTLV();
	break;
      case TLV_Cookie:
	tlv = new CookieTLV();
	break;
      }
      break;


    // ----- CHANNEL 2 -----
    case TLV_ParseMode_Channel02:
      switch(type) {
      case TLV_UserClass:
	tlv = new UserClassTLV();
	break;
      case TLV_SignupDate:
	tlv = new SignupDateTLV();
	break;
      case TLV_SignonDate:
	tlv = new SignonDateTLV();
	break;
      case TLV_Status:
	tlv = new StatusTLV();
	break;
      case TLV_WebAddress:
	tlv = new WebAddressTLV();
	break;
      case TLV_TimeOnline:
	tlv = new TimeOnlineTLV();
	break;
      case TLV_LANDetails:
	tlv = new LANDetailsTLV();
	break;
      case TLV_IPAddress:
	tlv = new IPAddressTLV();
	break;
      case TLV_Port:
	tlv = new PortTLV();
	break;
      }
      break;

      // ----- CHANNEL 3 -----
      // todo
      
      // ----- CHANNEL 4 -----
    case TLV_ParseMode_Channel04:
      switch(type) {
      case TLV_Screenname:
	tlv = new ScreenNameTLV();
	break;
      case TLV_Redirect:
	tlv = new RedirectTLV();
	break;
      case TLV_Cookie:
	tlv = new CookieTLV();
	break;
      case TLV_ErrorURL:
	tlv = new ErrorURLTLV();
	break;
      case TLV_ErrorCode:
	tlv = new ErrorCodeTLV();
	break;
      case TLV_DisconnectReason:
	tlv = new DisconnectReasonTLV();
	break;
      case TLV_DisconnectMessage:
	tlv = new DisconnectMessageTLV();
	break;
      }
      break;

      // ----- MESSAGEBLOCK -----
    case TLV_ParseMode_MessageBlock:
      switch(type) {
      case TLV_MessageData:
	tlv = new MessageDataTLV();
	break;
      case TLV_ICQData:
	tlv = new ICQDataTLV();
	break;
      }
      break;

      // ----- INMESSAGEDATA -----
    case TLV_ParseMode_InMessageData:
      switch(type) {
      case TLV_MessageText:
	tlv = new MessageTextTLV();
	break;
      }
      break;

    }

    if (tlv == NULL) {
      // unrecognised tlv
      // parse as a RawTLV
      tlv = new RawTLV(type);
    }

    tlv->ParseValue(b);

    return tlv;

  }

  void OutTLV::Output(Buffer& b) const {
    OutputHeader(b);
    OutputValue(b);
  }

  void OutTLV::OutputHeader(Buffer& b) const {
    b << Type();
  }

  // ----------------- Base Classes ---------------

  ShortTLV::ShortTLV() { }
  ShortTLV::ShortTLV(unsigned short n) : m_value(n) { }
  void ShortTLV::OutputValue(Buffer& b) const {
    b << Length();
    b << m_value;
  }
  void ShortTLV::ParseValue(Buffer& b) {
    unsigned short l;
    b >> l; // should be 2
    b >> m_value;
  }

  LongTLV::LongTLV() { }
  LongTLV::LongTLV(unsigned int n) : m_value(n) { }
  void LongTLV::OutputValue(Buffer& b) const {
    b << Length();
    b << m_value;
  }
  void LongTLV::ParseValue(Buffer& b) {
    unsigned short l;
    b >> l; // should be 4
    b >> m_value;
  }

  StringTLV::StringTLV() { }
  StringTLV::StringTLV(const string& val) : m_value(val) { }
  void StringTLV::OutputValue(Buffer& b) const {
    b << m_value;
  }
  void StringTLV::ParseValue(Buffer& b) {
    b >> m_value;
  }

  // ----------------- Actual Classes -------------

  // ----------------- ScreenName TLV -------------

  ScreenNameTLV::ScreenNameTLV() { }
  ScreenNameTLV::ScreenNameTLV(const string& val) : StringTLV(val) { }

  // ----------------- Password TLV ---------------
  
  PasswordTLV::PasswordTLV(const string& pw) : m_password(pw) { }
  void PasswordTLV::OutputValue(Buffer& b) const {
    b << (unsigned short)m_password.size();
    for(int i = 0; i < m_password.size(); i++)
      b << (unsigned char)(m_password[i] ^ XORtable[i%16]);

  }

  // ----------------- Status TLV -----------------

  void StatusTLV::OutputValue(Buffer& b) const {
    b << Length();
    b << m_allowDirect
      << m_webAware
      << m_status;
  }

  void StatusTLV::ParseValue(Buffer& b) {
    unsigned short l;
    b >> l; // should be 4
    b >> m_allowDirect
      >> m_webAware
      >> m_status;
  }

  // ----------------- Redirect TLV ---------------

  void RedirectTLV::ParseValue(Buffer& b) {
    string hp;
    b >> hp;

    int d = hp.find(':');
    if (d != -1) {
      m_server = hp.substr(0,d);
      m_port = atoi(hp.substr(d+1).c_str());
    }
  }

  // ----------------- Cookie TLV -----------------

  CookieTLV::CookieTLV(const unsigned char *ck, unsigned short len)
    : m_length(len)
  {
    m_value = new unsigned char[m_length];
    memcpy(m_value, ck, m_length);
  }

  CookieTLV::~CookieTLV() {
    if (m_value)
      delete [] m_value;
  }

  void CookieTLV::ParseValue(Buffer& b) {
    b >> m_length;

    m_value = new unsigned char[m_length];

    unsigned char c;
    for (unsigned short a = 0; a < m_length; a++) {
      b >> c;
      m_value[a] = c;
    }
  }

  void CookieTLV::OutputValue(Buffer& b) const {
    b  << m_length;
    for (unsigned short a = 0; a < m_length; a++)
      b << m_value[a];
  }

  // ----------------- LAN Details TLV ------------

  LANDetailsTLV::LANDetailsTLV()
    : m_firewall(0x0400), m_tcp_version(7) { }

  void LANDetailsTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;

    if (length == 0x0025) {
      // user accepts direct connections
      b >> m_lan_ip;
      b.advance(2);
      b >> m_lan_port;
    }

    unsigned int waste_int;
    unsigned short waste_short;
    b >> m_firewall
      >> m_tcp_version
      >> waste_int // unknown
      >> waste_int // always 0x00000050
      >> waste_int // always 0x00000003
      >> waste_int // unknown
      >> waste_int // unknown
      >> waste_int // unknown
      >> waste_short; // unknown
  }

  void LANDetailsTLV::OutputValue(Buffer& b) const {
    b << (unsigned short)0x001d;
    b << m_firewall
      << m_tcp_version
      << (unsigned int)0x00000000
      << (unsigned int)0x00000050
      << (unsigned int)0x00000003
      << (unsigned int)0x00000000
      << (unsigned int)0x00000000
      << (unsigned int)0x00000000
      << (unsigned short)0x0000;
  }

  // ----------------- Raw TLV --------------------

  RawTLV::RawTLV(unsigned short type) : m_type(type) { }


  void RawTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;

    unsigned char c;
    for(unsigned short a = 0; a < length; a++) {
      b >> c;
      m_value << c;
    }
  }

  void MessageDataTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;

    /*
     * A list of TLVs inside a TLV, these AOL
     * guys are craaazy..
     */
    TLVList tlvlist;
    tlvlist.Parse(b, TLV_ParseMode_InMessageData, (short unsigned int)-1);

    if (tlvlist.exists(TLV_MessageText))
      mttlv = *((MessageTextTLV*)tlvlist[TLV_MessageText]);

  }

  void MessageTextTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;
    b >> m_flag1;
    b >> m_flag2;

    b.Unpack(m_message, length-4);
    ICQSubType::CRLFtoLF(m_message);
  }

  ICQDataTLV::ICQDataTLV() : m_icqsubtype(NULL) { }

  ICQDataTLV::~ICQDataTLV() {
    if (m_icqsubtype != NULL) delete m_icqsubtype;
  }

  ICQSubType* ICQDataTLV::getICQSubType() const { return m_icqsubtype; }

  ICQSubType* ICQDataTLV::grabICQSubType() {
    ICQSubType *ret = m_icqsubtype;
    m_icqsubtype = NULL;
    return ret;
  }

  void ICQDataTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;

    /* Now this part you can see is where
     * the ICQ folks take over from the AOL folks
     * Intel byte ordering from now on..
     */
    b.setEndianness(Buffer::LITTLE);
    
    /*
     * UIN
     * For SMS - Magic UIN 1002
     */
    unsigned int uin;
    b >> uin;

    m_icqsubtype = ICQSubType::ParseICQSubType(b);
    
  }

  // ----------------- ICQSubtypes ----------------

  ICQSubType* ICQSubType::ParseICQSubType(Buffer& b) {
    unsigned short type;
    b >> type;

    ICQSubType *ist;
    switch(type) {
    case MSG_Type_Normal:
      ist = new NormalICQSubType(false);
      break;
    case MSG_Type_URL:
      ist = new URLICQSubType();
      break;
    case MSG_Type_SMS:
      ist = new SMSICQSubType();
      break;
    case MSG_Type_Multi:
      ist = new NormalICQSubType(true);
      break;
    default:
      throw ParseException("Unknown ICQ Subtype");
    }

    ist->Parse(b);

    return ist;
  }

  void ICQSubType::CRLFtoLF(string& s) {
    int curr = 0, next;
    while ( (next = s.find( "\r\n", curr )) != -1 ) {
      s.replace( next, 2, "\n" );
      curr = next + 1;
    }
  }

  void ICQSubType::LFtoCRLF(string& s) {
    int curr = 0, next;
    while ( (next = s.find( "\n", curr )) != -1 ) {
      s.replace( next, 1, "\r\n" );
      curr = next + 2;
    }
  }

  NormalICQSubType::NormalICQSubType(bool multi)
    : m_multi(multi) { }

  NormalICQSubType::NormalICQSubType(const string& msg, unsigned int uin)
    : m_message(msg), m_destination(uin) { }

  string NormalICQSubType::getMessage() const { return m_message; }
  
  bool NormalICQSubType::isMultiParty() const { return m_multi; }

  void NormalICQSubType::setMessage(const string& msg) { m_message = msg; }

  unsigned int NormalICQSubType::getDestination() const { return m_destination; }

  void NormalICQSubType::setDestination(unsigned int uin) { m_destination = uin; }

  void NormalICQSubType::Parse(Buffer& b) {
    unsigned short length;
    b >> length;
    b.Unpack(m_message, length-1);
    ICQSubType::CRLFtoLF(m_message);
    b.advance(1); // null terminator
  }

  unsigned short NormalICQSubType::getType() const { return MSG_Type_Normal; }

  URLICQSubType::URLICQSubType() { }

  URLICQSubType::URLICQSubType(const string& msg, const string& url, unsigned int source, unsigned int destination)
    : m_message(msg), m_url(url), m_source(source), m_destination(destination) { }

  string URLICQSubType::getMessage() const { return m_message; }

  string URLICQSubType::getURL() const { return m_url; }

  unsigned int URLICQSubType::getSource() const { return m_source; }

  unsigned int URLICQSubType::getDestination() const { return m_destination; }

  void URLICQSubType::setMessage(const string& msg) { m_message = msg; }

  void URLICQSubType::setURL(const string& url) { m_url = url; }

  void URLICQSubType::setSource(unsigned int uin) { m_source = uin; }

  void URLICQSubType::setDestination(unsigned int uin) { m_destination = uin; }

  void URLICQSubType::Parse(Buffer& b) {
    string text;
    unsigned short length;
    b >> length;
    b.Unpack(text, length-1);
    b.advance(1); // null terminator
    
    /*
     * Format is [message] 0xfe [url]
     */
    int l = text.find( 0xfe );
    if (l != -1) {
      m_message = text.substr( 0, l );
      m_url = text.substr( l+1 );
    } else {
      m_message = text;
      m_url = "";
    }
    ICQSubType::CRLFtoLF(m_message);
    ICQSubType::CRLFtoLF(m_url);

  }

  unsigned short URLICQSubType::getType() const { return MSG_Type_URL; }

  SMSICQSubType::SMSICQSubType() { }

  string SMSICQSubType::getMessage() const { return m_message; }

  SMSICQSubType::Type SMSICQSubType::getSMSType() const { return m_type; }

  void SMSICQSubType::Parse(Buffer& b) {
    /*
     * Here we go... this is a biggy
     */

    /* Next 21 bytes
     * Unknown 
     * 01 00 00 20 00 0e 28 f6 00 11 e7 d3 11 bc f3 00 04 ac 96 9d c2
     */
    b.advance(21);

    /* Delivery status
     *  0x0000 = SMS
     *  0x0002 = SMS Receipt Success
     *  0x0003 = SMS Receipt Failure
     */
    unsigned short del_stat;
    b >> del_stat;
    switch (del_stat) {
    case 0x0000:
      m_type = SMS;
      break;
    case 0x0002:
      m_type = SMS_Receipt_Success;
      break;
    case 0x0003:
      m_type = SMS_Receipt_Failure;
      break;
    default:
      // todo
      m_type = SMS;
    }

    /*
     * A Tag for the type, can be:
     * - "ICQSMS" NULL (?)
     * - "IrCQ-Net Invitation"
     * - ...
     * 07 00 00 00 49 43 51 53 4d 53 00
     * ---length-- ---string-----------
     */
    string tagstr;
    b.UnpackUint32String(tagstr);

    if (tagstr != string("ICQSMS")+'\0') {
      ostringstream ostr;
      ostr << "Unknown SNAC 0x0004 0x0007 ICQ SubType 0x001a tag string: " << tagstr;
      throw ParseException(ostr.str());
    }

    /* Next 3 bytes
     * Unknown
     * 00 00 00
     */
    b.advance(3);


    /* Length till end
     * 4 bytes
     */
    unsigned int msglen;
    b >> msglen;

    string xmlstr;
    b.UnpackUint32String(xmlstr);

    string::iterator s = xmlstr.begin();
    auto_ptr<XmlNode> top(XmlNode::parse(s, xmlstr.end()));

    if (top.get() == NULL) throw ParseException("Couldn't parse xml data in Message SNAC");

    if (m_type == SMS) {

      // -------- Normal SMS Message ---------
      if (top->getTag() != "sms_message") throw ParseException("No <sms_message> tag found in xml data");
      XmlBranch *sms_message = dynamic_cast<XmlBranch*>(top.get());
      if (sms_message == NULL || !sms_message->exists("text")) throw ParseException("No <text> tag found in xml data");
      XmlLeaf *text = sms_message->getLeaf("text");
      if (text == NULL) throw ParseException("<text> tag is not a leaf in xml data");
      m_message = text->getValue();
      
      /**
       * Extra fields
       * senders_network is always blank from my mobile
       */
      XmlLeaf *source = sms_message->getLeaf("source");
      if (source != NULL) m_source = source->getValue();

      XmlLeaf *sender = sms_message->getLeaf("sender");
      if (sender != NULL) m_sender = sender->getValue();

      XmlLeaf *senders_network = sms_message->getLeaf("senders_network");
      if (senders_network != NULL) m_senders_network = senders_network->getValue();

      XmlLeaf *time = sms_message->getLeaf("time");
      if (time != NULL) m_time = time->getValue();

      // ----------------------------------

    } else if (m_type == SMS_Receipt_Success) {

      // -- SMS Delivery Receipt Success --
      if (top->getTag() != "sms_delivery_receipt") throw ParseException("No <sms_delivery_receipt> tag found in xml data");
      XmlBranch *sms_rcpt = dynamic_cast<XmlBranch*>(top.get());
      if (sms_rcpt == NULL) throw ParseException("No tags found in <sms_delivery_receipt>");

      XmlLeaf *message_id = sms_rcpt->getLeaf("message_id");
      if (message_id != NULL) m_message_id = message_id->getValue();

      XmlLeaf *destination = sms_rcpt->getLeaf("destination");
      if (destination != NULL) m_destination = destination->getValue();

      XmlLeaf *delivered = sms_rcpt->getLeaf("delivered");
      m_delivered = false;
      if (delivered != NULL && delivered->getValue() == "Yes") m_delivered = true;

      XmlLeaf *text = sms_rcpt->getLeaf("text");
      if (text != NULL) m_message = text->getValue();

      XmlLeaf *submission_time = sms_rcpt->getLeaf("submition_time"); // can they not spell!
      if (submission_time != NULL) m_submission_time = submission_time->getValue();

      XmlLeaf *delivery_time = sms_rcpt->getLeaf("delivery_time");
      if (delivery_time != NULL) m_delivery_time = delivery_time->getValue();

      // ---------------------------------

    } else if (m_type == SMS_Receipt_Failure) {
      // todo
    }
      
  }

  unsigned short SMSICQSubType::getType() const { return MSG_Type_SMS; }

  // ----------------- TLV List -------------------

  TLVList::TLVList() { }
  TLVList::~TLVList() {
    // delete all elements from hash_map
    hash_map<unsigned short,InTLV*>::iterator i = tlvmap.begin();
    while (i != tlvmap.end()) {
      InTLV *t = (*i).second;
      delete t;
      i++;
    }
    tlvmap.clear();
  }

  void TLVList::Parse(Buffer& b, TLV_ParseMode pm, unsigned short no_tlvs) {
    InTLV *t;
    unsigned short ntlv = 0;
    while (b.beforeEnd() && ntlv < no_tlvs) {
      t = InTLV::ParseTLV(b,pm);
      // duplicate TLVs of one type - this shouldn't happen!
      if (tlvmap.count(t->Type())) {
	delete tlvmap[t->Type()];
      }

      tlvmap[t->Type()] = t;
      ntlv++;
    }
  }

  bool TLVList::exists(unsigned short type) {
    return (tlvmap.count(type) != 0);
  }
  
  InTLV* & TLVList::operator[](unsigned short type) {
    return tlvmap[type];
  }

}

Buffer& operator<<(Buffer& b, const ICQ2000::OutTLV& tlv) { tlv.Output(b); return b; }
