/*
 * SNAC - Messaging services
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

#include "SNAC-MSG.h"

#include "TLV.h"
#include "Contact.h"

namespace ICQ2000 {

  // --------------- Message (Family 0x0004) SNACs -----------------

  void MsgAddICBMParameterSNAC::OutputBody(Buffer& b) const {
    b << (unsigned int)0x00000000
      << (unsigned int)0x00031f40
      << (unsigned int)0x03e703e7
      << (unsigned int)0x00000000;
  }

  MsgSendSNAC::MsgSendSNAC(ICQSubType *icqsubtype, bool ad)
    : m_icqsubtype(icqsubtype), m_advanced(ad) { } 

  void MsgSendSNAC::setSeqNum(unsigned short seqnum) {
    m_seqnum = seqnum;
  }

  void MsgSendSNAC::setAdvanced(bool ad) {
    m_advanced = ad;
  }

  void MsgSendSNAC::OutputBody(Buffer& b) const {

    /*
     * ICBM Cookie - (we ignore it)
     */
    b << (unsigned int)0x00000000
      << (unsigned int)0x00000000;
    
    /* There is no consistency in the protocol here,
     * Messages are sent on channel 1
     * Advanced messages are sent on channel 2
     * Everything else on channel 4
     */
    if (m_advanced) {
      b << (unsigned short)0x0002;
      
      if (m_icqsubtype->getType() == MSG_Type_Normal) {
	NormalICQSubType *nst = static_cast<NormalICQSubType*>(m_icqsubtype);

	// Destination UIN (screenname)
	string sn = Contact::UINtoString(nst->getDestination());
	b << (unsigned char)sn.size();
	b.Pack(sn);
      } else if (m_icqsubtype->getType() == MSG_Type_URL) {
	URLICQSubType *ust = static_cast<URLICQSubType*>(m_icqsubtype);

	// Destination UIN (screenname)
	string sn = Contact::UINtoString(ust->getDestination());
	b << (unsigned char)sn.size();
	b.Pack(sn);
      }
      
      b << (unsigned short)0x0005;
      b << (unsigned short)(m_icqsubtype->Length() + 91);

      b << (unsigned short)0x0000     // status
	<< (unsigned int)  0x00000000 // cookie
	<< (unsigned int)  0x00000000 // cookie
	<< (unsigned int)  0x09461349 // cap
	<< (unsigned int)  0x4c7f11d1 // cap
	<< (unsigned int)  0x82224445 // cap
	<< (unsigned int)  0x53540000;// cap

      b << (unsigned short)0x000a  // TLV
	<< (unsigned short)0x0002
	<< (unsigned short)0x0001;

      b << (unsigned short)0x000f  // TLV
	<< (unsigned short)0x0000;

      b << (unsigned short)0x2711  // TLV
	<< (unsigned short)(m_icqsubtype->Length() + 51);
      

      // unknown..
      b << (unsigned int)  0x1B000700
	<< (unsigned int)  0x00000000
	<< (unsigned int)  0x00000000
	<< (unsigned int)  0x00000000
	<< (unsigned int)  0x00000000
	<< (unsigned int)  0x00000300
	<< (unsigned short)0x0000;

      b << (unsigned char) 0x00;

      b.setEndianness(Buffer::LITTLE);
      b << m_seqnum
	<< (unsigned short)0x000e
	<< m_seqnum;

      // unknown
      b << (unsigned int)0x00000000
	<< (unsigned int)0x00000000
	<< (unsigned int)0x00000000;

      m_icqsubtype->Output(b, m_advanced);
      
      b.setEndianness(Buffer::BIG);
      b << (unsigned short)0x0003
	<< (unsigned short)0x0000;

      return;
    }

    // non-advanced

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
     * Channel 0x0002 = Advanced messages
     * Channel 0x0004 = ICQ specific features (URLs, Added to Contact List, SMS, etc.)
     */
    unsigned short channel;
    b >> channel;
    if (channel != 0x0001 && channel != 0x0002 && channel != 0x0004)
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
    if (channel == 0x0001) {
      TLVList tlvlist;
      tlvlist.Parse(b, TLV_ParseMode_MessageBlock, (unsigned int)-1);

      // Normal message
      if (!tlvlist.exists(TLV_MessageData))
	throw ParseException("No message data in SNAC");

      MessageDataTLV *t = static_cast<MessageDataTLV*>(tlvlist[TLV_MessageData]);
      // coerce this into the NormalICQSubType
      NormalICQSubType *nst = new NormalICQSubType(false);
      nst->setMessage( t->getMessage() );
      m_icqsubtype = nst;

    } else if (channel == 0x0002) {
      TLVList tlvlist;
      tlvlist.Parse(b, TLV_ParseMode_AdvMsgBlock, (unsigned int)-1);
      
      if (!tlvlist.exists(TLV_AdvMsgData))
	throw ParseException("No Advanced Message TLV in SNAC 0x0004 0x0007 on channel 2");

      AdvMsgDataTLV *t = static_cast<AdvMsgDataTLV*>(tlvlist[TLV_AdvMsgData]);
      m_icqsubtype = t->grabICQSubType();

    } else if (channel == 0x0004) {
      TLVList tlvlist;
      tlvlist.Parse(b, TLV_ParseMode_MessageBlock, (unsigned int)-1);

      /* ICQ hacked in messages
       * - SMS message
       * - URLs
       * - Added to contactlist
       * ..
       */

      if (!tlvlist.exists(TLV_ICQData))
	throw ParseException("No ICQ data TLV in SNAC 0x0004 0x0007 on channel 4");
      
      ICQDataTLV *t = static_cast<ICQDataTLV*>(tlvlist[TLV_ICQData]);
      m_icqsubtype = t->grabICQSubType();

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

}