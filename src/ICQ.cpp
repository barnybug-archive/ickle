/*
 * ICQ Subtypes
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

#include "ICQ.h"

namespace ICQ2000 {

  // ----------------- ICQSubtypes ----------------

  ICQSubType::ICQSubType()
    : m_flags(0x0000) { }

  ICQSubType* ICQSubType::ParseICQSubType(Buffer& b, bool adv) {
    unsigned char type, flags;
    b >> type
      >> flags;
    
    bool multi = (flags & MSG_Flag_Multi);

    if (adv) {
      unsigned short m_flag1, m_flag2;
      b >> m_flag1
	>> m_flag2;
    }

    ICQSubType *ist;
    switch(type) {
    case MSG_Type_Normal:
      ist = new NormalICQSubType(multi, adv);
      break;
    case MSG_Type_URL:
      ist = new URLICQSubType(adv);
      break;
    case MSG_Type_SMS:
      ist = new SMSICQSubType();
      break;
    case MSG_Type_AutoReq_Away:
    case MSG_Type_AutoReq_Occ:
    case MSG_Type_AutoReq_NA:
    case MSG_Type_AutoReq_DND:
    case MSG_Type_AutoReq_FFC:
      ist = new ReqAwayICQSubType(type);
      break;
    default:
      throw ParseException("Unknown ICQ Subtype");
    }

    ist->setFlags(flags);
    ist->Parse(b);

    return ist;
  }

  void ICQSubType::Output(Buffer& b) const {
    b << getType()
      << getFlags();

    OutputBody(b);
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

  UINRelatedSubType::UINRelatedSubType(bool adv)
    : m_source(0), m_destination(0), m_advanced(adv), m_ack(false) { }

  UINRelatedSubType::UINRelatedSubType(unsigned int s, unsigned int d, bool adv)
    : m_source(s), m_destination(d), m_advanced(adv), m_ack(false)  { }

  unsigned int UINRelatedSubType::getSource() const { return m_source; }

  unsigned int UINRelatedSubType::getDestination() const { return m_destination; }

  void UINRelatedSubType::setDestination(unsigned int d) { m_destination = d; }

  void UINRelatedSubType::setSource(unsigned int s) { m_source = s; }

  bool UINRelatedSubType::isAdvanced() const { return m_advanced; }

  void UINRelatedSubType::setAdvanced(bool b) { m_advanced = b; }

  bool UINRelatedSubType::isACK() const { return m_ack; }

  void UINRelatedSubType::setACK(bool b) { m_ack = b; }

  NormalICQSubType::NormalICQSubType(bool multi, bool adv)
    : UINRelatedSubType(adv), m_multi(multi), m_foreground(0x00000000),
      m_background(0x00ffffff) { }

  NormalICQSubType::NormalICQSubType(const string& msg, unsigned int uin, bool adv)
    : UINRelatedSubType(0, uin, adv), m_message(msg), m_foreground(0x00000000),
      m_background(0x00ffffff) { }

  string NormalICQSubType::getMessage() const { return m_message; }
  
  bool NormalICQSubType::isMultiParty() const { return m_multi; }

  void NormalICQSubType::setMessage(const string& msg) { m_message = msg; }

  void NormalICQSubType::Parse(Buffer& b) {
    b.UnpackUint16StringNull(m_message);
    ICQSubType::CRLFtoLF(m_message);

    if (m_advanced) {
      b >> m_foreground
	>> m_background;
    } else {
      m_foreground = 0x00000000;
      m_background = 0x00ffffff;
    }
  }

  void NormalICQSubType::OutputBody(Buffer& b) const {
    if (m_advanced) {
      b << (unsigned short)0x0000
	<< (unsigned short)(m_ack ? 0x0000: 0x0001);
    }

    if (m_ack) {
      b.PackUint16StringNull("");
    } else {
      string m_text = m_message;
      ICQSubType::LFtoCRLF(m_text);
      b.PackUint16StringNull(m_text);
    }
    
    if (m_advanced) {
      if (m_ack) {
	b << (unsigned int)0x00000000
	  << (unsigned int)0xffffffff;
      } else {
	b << (unsigned int)m_foreground
	  << (unsigned int)m_background;
      }
    }
  }

  unsigned short NormalICQSubType::Length() const {
    string text = m_message;
    ICQSubType::LFtoCRLF(text);

    return text.size() + (m_advanced ? 13 : 5);
  }

  unsigned char NormalICQSubType::getType() const { return MSG_Type_Normal; }

  void NormalICQSubType::setForeground(unsigned int f) { m_foreground = f; }

  void NormalICQSubType::setBackground(unsigned int b) { m_background = b; }
  
  unsigned int NormalICQSubType::getForeground() const { return m_foreground; }

  unsigned int NormalICQSubType::getBackground() const { return m_background; }

  URLICQSubType::URLICQSubType(bool adv)
    : UINRelatedSubType(adv) { }

  URLICQSubType::URLICQSubType(const string& msg, const string& url, unsigned int source, unsigned int destination, bool adv)
    : m_message(msg), m_url(url), UINRelatedSubType(source, destination, adv) { }

  string URLICQSubType::getMessage() const { return m_message; }

  string URLICQSubType::getURL() const { return m_url; }

  void URLICQSubType::setMessage(const string& msg) { m_message = msg; }

  void URLICQSubType::setURL(const string& url) { m_url = url; }

  void URLICQSubType::Parse(Buffer& b) {
    string text;
    b.UnpackUint16StringNull(text);
    
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

  void URLICQSubType::OutputBody(Buffer& b) const {
    if (m_advanced) {
      b << (unsigned short)0x0000
	<< (unsigned short)0x0001;
    }

    if (m_ack) {
      b.PackUint16StringNull("");
    } else {
      ostringstream ostr;
      ostr << m_message << (unsigned char)0xfe << m_url;
      string m_text = ostr.str();
      ICQSubType::LFtoCRLF(m_text);
      b.PackUint16StringNull(m_text);
    }
  }

  unsigned short URLICQSubType::Length() const {
    string text = m_message + m_url;
    ICQSubType::LFtoCRLF(text);

    return text.size() + 6;
  }

  unsigned char URLICQSubType::getType() const { return MSG_Type_URL; }

  ReqAwayICQSubType::ReqAwayICQSubType(unsigned char type)
   : UINRelatedSubType(true), m_type(type) { }

  ReqAwayICQSubType::ReqAwayICQSubType(Status s, unsigned int uin)
    : UINRelatedSubType(0, uin, true) {

    switch(s) {
    case STATUS_AWAY:
      m_type = MSG_Type_AutoReq_Away;
      break;
    case STATUS_OCCUPIED:
      m_type = MSG_Type_AutoReq_Occ;
      break;
    case STATUS_NA:
      m_type = MSG_Type_AutoReq_NA;
      break;
    case STATUS_DND:
      m_type = MSG_Type_AutoReq_DND;
      break;
    case STATUS_FREEFORCHAT:
      m_type = MSG_Type_AutoReq_FFC;
      break;
    default:
      m_type = MSG_Type_AutoReq_Away;
    }

  }

  void ReqAwayICQSubType::Parse(Buffer& b) {
    b.UnpackUint16StringNull(m_message);
    ICQSubType::CRLFtoLF(m_message);
  }

  void ReqAwayICQSubType::OutputBody(Buffer& b) const {
    b << (unsigned short)0x0000
      << (unsigned short)0x0001;

    // empty message
    b << (unsigned short)0x0001
      << (unsigned char)0x00;
  }

  unsigned short ReqAwayICQSubType::Length() const {
    return 9;
  }

  unsigned char ReqAwayICQSubType::getType() const { return m_type; }

  unsigned char ReqAwayICQSubType::getFlags() const { return 0x03; }

  string ReqAwayICQSubType::getMessage() const { return m_message; }

  void ReqAwayICQSubType::setMessage(const string& msg) { m_message = msg; }

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

  void SMSICQSubType::OutputBody(Buffer& b) const {  }

  unsigned short SMSICQSubType::Length() const { return 0; }

  unsigned char SMSICQSubType::getType() const { return MSG_Type_SMS; }

  
}
