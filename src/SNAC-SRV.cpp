/*
 * SNAC - Server
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

#include "SNAC-SRV.h"

#include "TLV.h"
#include "Xml.h"
#include "Contact.h"

namespace ICQ2000 {

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
    m_icqsubtype = ICQSubType::ParseICQSubType(b, false);
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

}