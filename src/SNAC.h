/*
 * SNACs 
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

#ifndef SNAC_H
#define SNAC_H

#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <time.h>
#include <list>

#include "buffer.h"
#include "TLV.h"
#include "Xml.h"
#include "Contact.h"
#include "ContactList.h"
#include "exceptions.h"

using namespace std;

namespace ICQ2000 {
 
  // ------------- SNAC numerical constants ------------

  // SNAC Families
  const unsigned short SNAC_FAM_GEN = 0x0001;
  const unsigned short SNAC_FAM_LOC = 0x0002;
  const unsigned short SNAC_FAM_BUD = 0x0003;
  const unsigned short SNAC_FAM_MSG = 0x0004;
  const unsigned short SNAC_FAM_ADS = 0x0005;
  const unsigned short SNAC_FAM_INV = 0x0006;
  const unsigned short SNAC_FAM_ADM = 0x0007;
  const unsigned short SNAC_FAM_POP = 0x0008;
  const unsigned short SNAC_FAM_BOS = 0x0009;
  const unsigned short SNAC_FAM_LUP = 0x000a;
  const unsigned short SNAC_FAM_STS = 0x000b;
  const unsigned short SNAC_FAM_TRT = 0x000c;
  const unsigned short SNAC_FAM_CNV = 0x000d;
  const unsigned short SNAC_FAM_CHT = 0x000e;

  const unsigned short SNAC_FAM_SRV = 0x0015; // Server messages

  // Generic (Family 0x0001)
  const unsigned short SNAC_GEN_Error = 0x0001;
  const unsigned short SNAC_GEN_ClientReady = 0x0002;
  const unsigned short SNAC_GEN_ServerReady = 0x0003;
  const unsigned short SNAC_GEN_NewService = 0x0004;
  const unsigned short SNAC_GEN_Redirect = 0x0005;
  const unsigned short SNAC_GEN_RateInfoRequest = 0x0006;
  const unsigned short SNAC_GEN_RateInfo = 0x0007;
  const unsigned short SNAC_GEN_RateInfoAck = 0x0008;
  const unsigned short SNAC_GEN_RateInfoChange = 0x000a;
  const unsigned short SNAC_GEN_ServerPause = 0x000b;
  const unsigned short SNAC_GEN_ServerResume = 0x000d;

  const unsigned short SNAC_GEN_UserInfoRequest = 0x000e;
  const unsigned short SNAC_GEN_UserInfo = 0x000f;
  const unsigned short SNAC_GEN_Evil = 0x0010; // ??
  const unsigned short SNAC_GEN_SetIdle = 0x0011; // maybe AIM specific?
  const unsigned short SNAC_GEN_MigrationRequest = 0x0012; // maybe AIM specific?
  const unsigned short SNAC_GEN_MOTD = 0x0013;
  const unsigned short SNAC_GEN_SetPrivFlags = 0x0014;
  const unsigned short SNAC_GEN_WellKnownURL = 0x0015; // ??
  const unsigned short SNAC_GEN_NOP = 0x0016;

  // The next lot seem to be ICQ specific
  const unsigned short SNAC_GEN_Capabilities = 0x0017;
  const unsigned short SNAC_GEN_CapAck = 0x0018;
  const unsigned short SNAC_GEN_SetStatus = 0x001e;


  // Locate (Family 0x0002)
  const unsigned short SNAC_LOC_Error = 0x0001;
  const unsigned short SNAC_LOC_RightsReq = 0x0002;
  const unsigned short SNAC_LOC_Rights = 0x0003;

  // Buddy stuff (Family 0x0003)
  const unsigned short SNAC_BUD_Error = 0x0001;
  const unsigned short SNAC_BUD_AddBuddy = 0x0004;
  const unsigned short SNAC_BUD_RemoveBuddy = 0x0005;
  const unsigned short SNAC_BUD_Online = 0x000b;
  const unsigned short SNAC_BUD_Offline = 0x000c;

  // Messages (Family 0x0004)
  const unsigned short SNAC_MSG_Error = 0x0001;
  const unsigned short SNAC_MSG_Send = 0x0006;
  const unsigned short SNAC_MSG_Message = 0x0007;

  // Server Messages (Family 0x0015) - messages through the server
  const unsigned short SNAC_SRV_Error = 0x0001;
  const unsigned short SNAC_SRV_Send = 0x0002;
  const unsigned short SNAC_SRV_Response = 0x0003;
  /*
   * SRV_Response is very generic - ICQ have hacked all the extra ICQ
   * functionality into this one
   */
  

  // ---------------- Other Stuff ---------------------------------

  /* the user information block of screenname and then TLVs
   * of user info appears in several different SNACs so
   * encapsulate it here
   */
  class UserInfoBlock {
   protected:
    string m_screenname;
    unsigned short m_warninglevel, m_userClass;
    unsigned char m_allowDirect, m_webAware;
    unsigned short m_status;
    unsigned int m_timeOnline;
    unsigned int m_signupDate, m_signonDate;
    unsigned int m_lan_ip, m_ext_ip;
    unsigned short m_lan_port, m_ext_port, m_firewall;
    unsigned char m_tcp_version;
    
   public:
    UserInfoBlock() { }

    string getScreenName() const;
    unsigned int getUIN() const;
    unsigned int getTimeOnline() const;
    unsigned int getLanIP() const;
    unsigned int getExtIP() const;
    unsigned short getLanPort() const;
    unsigned short getExtPort() const;
    unsigned short getFirewall() const;
    unsigned char getTCPVersion() const;
    unsigned short getStatus() const;

    void Parse(Buffer& b);
  };

  
  // ------------- abstract SNAC classes ---------------

  class SNAC {
   public:
    virtual ~SNAC() { }
    
    virtual unsigned short Family() const = 0;
    virtual unsigned short Subtype() const = 0;
    virtual unsigned short Flags() const = 0;
    virtual unsigned int RequestID() const = 0;
  };

  // -- Inbound SNAC --
  class InSNAC : virtual public SNAC {
   protected:
    unsigned short m_flags;
    unsigned int m_requestID;
    
    virtual void ParseBody(Buffer& b) = 0;

   public:
    virtual void Parse(Buffer& b);

    virtual unsigned short Flags() const;
    virtual unsigned int RequestID() const;

    static InSNAC* ParseSNAC(Buffer& b);
  };

  // -- Outbound SNAC --
  class OutSNAC : virtual public SNAC {
   protected:
    virtual void OutputHeader(Buffer& b) const;
    virtual void OutputBody(Buffer& b) const = 0;

   public:
    unsigned short Flags() const { return 0x0000; }
    unsigned int RequestID() const { return 0x00000000; }

    virtual void Output(Buffer& b) const;
  };

  // ------------ Raw SNAC ----------------------------------
  
  class RawSNAC : public InSNAC {
   protected:
    unsigned short m_family, m_subtype;
    Buffer m_data;

    void ParseBody(Buffer& b);
    
   public:
    RawSNAC(unsigned short f, unsigned short t);

    unsigned short Family() const { return m_family; }
    unsigned short Subtype() const { return m_subtype; }
  };

  // ------------ Generic (Family 0x0001) SNACs -------------
  
  class GenericSNAC : virtual public SNAC {
   public:
    unsigned short Family() const { return SNAC_FAM_GEN; }
  };

  class ServerReadySNAC : public GenericSNAC, public InSNAC {
   protected:
    void ParseBody(Buffer& b);
    
   public:
    ServerReadySNAC() { }

    unsigned short Subtype() const { return SNAC_GEN_ServerReady; }
  };

  class CapabilitiesSNAC : public GenericSNAC, public OutSNAC {
   protected:
    void OutputBody(Buffer& b) const;

   public:
    CapabilitiesSNAC() { }
    unsigned short Subtype() const { return SNAC_GEN_Capabilities; }
  };

  class CapAckSNAC : public GenericSNAC, public InSNAC {
   protected:
    void ParseBody(Buffer& b);
    
   public:
    CapAckSNAC() { }

    unsigned short Subtype() const { return SNAC_GEN_CapAck; }
  };

  class SetStatusSNAC : public GenericSNAC, public OutSNAC {
   private:
    unsigned short m_status;
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    SetStatusSNAC(unsigned short status);
    unsigned short Subtype() const { return SNAC_GEN_SetStatus; }
  };

  class SetIdleSNAC : public GenericSNAC, public OutSNAC {
   protected:
    void OutputBody(Buffer& b) const;

   public:
    SetIdleSNAC() { }
    unsigned short Subtype() const { return SNAC_GEN_SetIdle; }
  };

  class ClientReadySNAC : public GenericSNAC, public OutSNAC {
   protected:
    void OutputBody(Buffer& b) const;

   public:
    ClientReadySNAC() { }
    unsigned short Subtype() const { return SNAC_GEN_ClientReady; }
  };

  class UserInfoSNAC : public GenericSNAC, public InSNAC {
   private:
    UserInfoBlock m_userinfo;

   protected:
    void ParseBody(Buffer& b);
    
   public:
    UserInfoSNAC() { }

    unsigned short Subtype() const { return SNAC_GEN_UserInfo; }
  };

  const unsigned char MOTD_MANDATORY_UPGRADE = 0x01; // hehe - like we're going to obey this :-)
  const unsigned char MOTD_ADVISORY_UPGRADE = 0x02;
  const unsigned char MOTD_SYSTEM_BULLETIN = 0x03;
  const unsigned char MOTD_NORMAL = 0x04;

  class MOTDSNAC : public GenericSNAC, public InSNAC {
   private:
    unsigned char m_status;
    string m_url;

   protected:
    void ParseBody(Buffer& b);
    
   public:
    MOTDSNAC() { }

    unsigned short Subtype() const { return SNAC_GEN_MOTD; }
  };

  // ----------------- Buddy List (Family 0x0003) SNACs -----------

  class BUDFamilySNAC : virtual public SNAC {
   public:
    unsigned short Family() const { return SNAC_FAM_BUD; }
  };

  class AddBuddySNAC : public BUDFamilySNAC, public OutSNAC {
   private:
    list<string> m_buddy_list;
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    AddBuddySNAC();
    AddBuddySNAC(const ContactList& l);
    AddBuddySNAC(const Contact& c);

    void addBuddy(const Contact& c);

    unsigned short Subtype() const { return SNAC_BUD_AddBuddy; }
  };

  class RemoveBuddySNAC : public BUDFamilySNAC, public OutSNAC {
   private:
    list<string> m_buddy_list;
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    RemoveBuddySNAC();
    RemoveBuddySNAC(const ContactList& l);
    RemoveBuddySNAC(const string& s);

    void removeBuddy(const Contact& c);

    unsigned short Subtype() const { return SNAC_BUD_RemoveBuddy; }
  };

  class BuddyOnlineSNAC : public BUDFamilySNAC, public InSNAC {
   private:
    UserInfoBlock m_userinfo;

  protected:
    void ParseBody(Buffer& b);

   public:
    BuddyOnlineSNAC();

    const UserInfoBlock& getUserInfo() const { return m_userinfo; }
    unsigned short Subtype() const { return SNAC_BUD_Online; }
  };
  

  class BuddyOfflineSNAC : public BUDFamilySNAC, public InSNAC {
   private:
    UserInfoBlock m_userinfo;

  protected:
    void ParseBody(Buffer& b);

   public:
    BuddyOfflineSNAC();

    UserInfoBlock getUserInfo() const { return m_userinfo; }
    unsigned short Subtype() const { return SNAC_BUD_Offline; }
  };
  

  // ----------------- Message (Family 0x0004) SNACs --------------
  
  class MsgFamilySNAC : virtual public SNAC {
   public:
    unsigned short Family() const { return SNAC_FAM_MSG; }
  };

  class MsgSendSNAC : public MsgFamilySNAC, public OutSNAC {
   protected:
    ICQSubType *m_icqsubtype;

    void OutputBody(Buffer& b) const;

   public:
    MsgSendSNAC(ICQSubType *icqsubtype);

    unsigned short Subtype() const { return SNAC_MSG_Send; }
  };

  class MessageSNAC : public MsgFamilySNAC, public InSNAC {
   protected:

    // General fields
    UserInfoBlock m_userinfo;
    ICQSubType *m_icqsubtype;

    void ParseBody(Buffer& b);

   public:
    MessageSNAC();
    ~MessageSNAC();

    ICQSubType* getICQSubType() const { return m_icqsubtype; }
    UserInfoBlock getUserInfo() const { return m_userinfo; }

    unsigned short Subtype() const { return SNAC_MSG_Message; }
  };

  // --------------------- Server (Family 0x0015) SNACs ---------

  class SrvFamilySNAC : virtual public SNAC {
   public:
    unsigned short Family() const { return SNAC_FAM_SRV; }
  };

  class SrvSendSNAC : public SrvFamilySNAC, public OutSNAC {
   protected:
    string m_text, m_destination, m_senders_name;
    unsigned int m_senders_UIN;
    bool m_delivery_receipt;
    
    void OutputBody(Buffer& b) const;

   public:
    SrvSendSNAC(const string& text, const string& destination,
		unsigned int senders_UIN, const string& senders_name, bool delrpt);

    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvRequestOfflineSNAC : public SrvFamilySNAC, public OutSNAC {
   private:
    unsigned int m_uin;

   protected:
    void OutputBody(Buffer& b) const;

   public:
    SrvRequestOfflineSNAC(unsigned int uin);

    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvAckOfflineSNAC : public SrvFamilySNAC, public OutSNAC {
   private:
    unsigned int m_uin;

   protected:
    void OutputBody(Buffer& b) const;

   public:
    SrvAckOfflineSNAC(unsigned int uin);

    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvRequestSimpleUserInfo : public SrvFamilySNAC, public OutSNAC {
   private:
    unsigned int m_my_uin, m_user_uin;

   protected:
    void OutputBody(Buffer& b) const;

   public:
    SrvRequestSimpleUserInfo(unsigned int my_uin, unsigned int user_uin);
    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvResponseSNAC : public SrvFamilySNAC, public InSNAC {
   public:
    enum ResponseType {
      OfflineMessage,
      OfflineMessagesComplete,
      SMS_Error,
      SMS_Response,
      SimpleUserInfo
    };

   protected:
    ResponseType m_type;

    // SMS Response fields
    string m_source, m_network, m_message_id, m_messages_left;
    bool m_deliverable;
    int m_error_id;
    string m_error_param;

    // Offline Message fields
    time_t m_time;
    unsigned int m_sender_UIN;
    ICQSubType *m_icqsubtype;

    // SimpleUserInfo fields
    unsigned int m_uin;
    string m_alias, m_first_name, m_last_name, m_email;
    bool m_authreq;
    unsigned char m_status;

    void ParseBody(Buffer& b);
    void ParseICQResponse(Buffer& b);
    void ParseOfflineMessage(Buffer& b);
    void ParseSMSError(Buffer& b);
    void ParseSMSResponse(Buffer& b);
    void ParseSimpleUserInfo(Buffer &b);
    
   public:
    SrvResponseSNAC();
    ~SrvResponseSNAC();

    ResponseType getType() const { return m_type; }
    string getSource() const { return m_source; }
    bool deliverable() const { return m_deliverable; }
    string getNetwork() const { return m_network; }
    string getMessageId() const { return m_message_id; }
    string getMessagesLeft() const { return m_messages_left; }
    int getErrorId() const { return m_error_id; }
    string getErrorParam() const { return m_error_param; }

    ICQSubType *getICQSubType() const { return m_icqsubtype; }
    unsigned int getSenderUIN() const { return m_sender_UIN; }
    time_t getTime() const { return m_time; }

    unsigned int getUIN() const { return m_uin; }
    string getAlias() const { return m_alias; }
    string getFirstName() const { return m_first_name; }
    string getLastName() const { return m_last_name; }
    string getEmail() const { return m_email; }

    unsigned short Subtype() const { return SNAC_SRV_Response; }
  };

}

Buffer& operator<<(Buffer& b, const ICQ2000::OutSNAC& t);

#endif
