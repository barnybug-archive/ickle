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
      ist = new NormalICQSubType(multi);
      break;
    case MSG_Type_URL:
      ist = new URLICQSubType();
      break;
    case MSG_Type_SMS:
      ist = new SMSICQSubType();
      break;
    default:
      throw ParseException("Unknown ICQ Subtype");
    }

    ist->Parse(b, adv);

    return ist;
  }

  void ICQSubType::Output(Buffer& b, bool adv) const {
    b << getType()
      << getFlags();

    if (adv) {
      b << (unsigned short)0x0000
	<< (unsigned short)0x0001;
    }

    OutputBody(b, adv);
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
    : m_multi(multi), m_foreground(0x00000000),
      m_background(0x00ffffff) { }

  NormalICQSubType::NormalICQSubType(const string& msg, unsigned int uin)
    : m_message(msg), m_destination(uin), m_foreground(0x00000000),
      m_background(0x00ffffff) { }

  string NormalICQSubType::getMessage() const { return m_message; }
  
  bool NormalICQSubType::isMultiParty() const { return m_multi; }

  void NormalICQSubType::setMessage(const string& msg) { m_message = msg; }

  unsigned int NormalICQSubType::getDestination() const { return m_destination; }

  void NormalICQSubType::setDestination(unsigned int uin) { m_destination = uin; }

  void NormalICQSubType::Parse(Buffer& b, bool adv) {
    unsigned short length;
    b >> length;
    b.Unpack(m_message, length-1);
    ICQSubType::CRLFtoLF(m_message);
    b.advance(1); // null terminator

    if (adv) {
      b >> m_foreground
	>> m_background;
    } else {
      m_foreground = 0x00000000;
      m_background = 0x00ffffff;
    }
  }

  void NormalICQSubType::OutputBody(Buffer& b, bool adv) const {
    string m_text = m_message;
    ICQSubType::LFtoCRLF(m_text);
    b.PackUint16StringNull(m_text);
    
    b << (unsigned int)m_foreground
      << (unsigned int)m_background;
    
  }

  unsigned short NormalICQSubType::Length() const { return m_message.size() + 11; }

  unsigned char NormalICQSubType::getType() const { return MSG_Type_Normal; }

  void NormalICQSubType::setForeground(unsigned int f) { m_foreground = f; }

  void NormalICQSubType::setBackground(unsigned int b) { m_background = b; }
  
  unsigned int NormalICQSubType::getForeground() const { return m_foreground; }

  unsigned int NormalICQSubType::getBackground() const { return m_background; }

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

  void URLICQSubType::Parse(Buffer& b, bool adv) {
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

  void URLICQSubType::OutputBody(Buffer& b, bool adv) const {
    ostringstream ostr;
    ostr << m_message << (unsigned char)0xfe << m_url;
    string m_text = ostr.str();
    ICQSubType::LFtoCRLF(m_text);
    b.PackUint16StringNull(m_text);
  }

  unsigned short URLICQSubType::Length() const { return m_message.size() + m_url.size() + 4; }

  unsigned char URLICQSubType::getType() const { return MSG_Type_URL; }

  SMSICQSubType::SMSICQSubType() { }

  string SMSICQSubType::getMessage() const { return m_message; }

  SMSICQSubType::Type SMSICQSubType::getSMSType() const { return m_type; }

  void SMSICQSubType::Parse(Buffer& b, bool adv) {
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

  void SMSICQSubType::OutputBody(Buffer& b, bool adv) const {  }

  unsigned short SMSICQSubType::Length() const { return 0; }

  unsigned char SMSICQSubType::getType() const { return MSG_Type_SMS; }

  
}
