/* $Id: Icons.cpp,v 1.17 2002-07-21 00:23:37 bugcreator Exp $
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

#include "Icons.h"
#include "Settings.h"
#include "main.h"

#include "online.xpm"
#include "offline.xpm"
#include "away.xpm"
#include "na.xpm"
#include "ffc.xpm"
#include "dnd.xpm"
#include "occ.xpm"
//#include "chat.xpm"
#include "invisible.xpm"
#include "message.xpm"
#include "url.xpm"
#include "sms.xpm"
//#include "file.xpm"
#include "sysmsg.xpm"

using std::string;

using ICQ2000::Status;

Icons::Icons()
{
}

/*
 * Sets the icons to the default compiled in icons.
 */
void Icons::setDefaultIcons() {
  Icon_Status_Online.reset( new Gtk::ImageLoaderData(online_xpm) );
  Icon_Status_Away.reset( new Gtk::ImageLoaderData(away_xpm) );
  Icon_Status_NA.reset( new Gtk::ImageLoaderData(na_xpm) );
  Icon_Status_Occupied.reset( new Gtk::ImageLoaderData(occ_xpm) );
  Icon_Status_DND.reset( new Gtk::ImageLoaderData(dnd_xpm) );
  Icon_Status_FFC.reset( new Gtk::ImageLoaderData(ffc_xpm) );
  Icon_Status_Offline.reset( new Gtk::ImageLoaderData(offline_xpm) );
  Icon_Status_Message.reset( new Gtk::ImageLoaderData(message_xpm) );
  Icon_Status_URL.reset( new Gtk::ImageLoaderData(url_xpm) );
  Icon_Status_SMS.reset( new Gtk::ImageLoaderData(sms_xpm) );
  Icon_Status_SystemMessage.reset( new Gtk::ImageLoaderData(sysmsg_xpm) );
  Icon_Status_Invisible.reset( new Gtk::ImageLoaderData(invisible_xpm) );
}

void Icons::settings_changed_cb(const string& key) {
  if (key == "icons_dir") {
    setIcons( g_settings.getValueString("icons_dir") );
  }
}

bool Icons::setIcons(const string &dir) {

  if (dir == "" || dir == "Default") {
    setDefaultIcons();
    icons_changed.emit();
    return true;
  }

  Icon_Status_Online.reset( new Gtk::ImageLoader( dir + "online.xpm" ) );
  Icon_Status_Away.reset( new Gtk::ImageLoader( dir + "away.xpm" ) );
  Icon_Status_NA.reset( new Gtk::ImageLoader( dir + "na.xpm" ) );
  Icon_Status_Occupied.reset( new Gtk::ImageLoader( dir + "occ.xpm" ) );
  Icon_Status_DND.reset( new Gtk::ImageLoader( dir + "dnd.xpm" ) );
  Icon_Status_FFC.reset( new Gtk::ImageLoader( dir + "ffc.xpm" ) );
  Icon_Status_Offline.reset( new Gtk::ImageLoader( dir + "offline.xpm" ) );
  Icon_Status_Message.reset( new Gtk::ImageLoader( dir + "message.xpm" ) );
  Icon_Status_URL.reset( new Gtk::ImageLoader( dir + "url.xpm" ) );
  Icon_Status_SMS.reset( new Gtk::ImageLoader( dir + "sms.xpm" ) );
  Icon_Status_SystemMessage.reset( new Gtk::ImageLoader( dir + "sysmsg.xpm" ) );
  Icon_Status_Invisible.reset( new Gtk::ImageLoader( dir + "invisible.xpm" ) );
  icons_changed.emit();
  return true;
}

Gtk::ImageLoader* Icons::IconForStatus(Status s, bool inv) { 
  Gtk::ImageLoader *p;
  if (inv && s != ICQ2000::STATUS_OFFLINE) {
    p = Icon_Status_Invisible.get();
  } else {
    switch(s) {
    case ICQ2000::STATUS_ONLINE:
      p = Icon_Status_Online.get();
      break;
    case ICQ2000::STATUS_AWAY:
      p = Icon_Status_Away.get();
      break;
    case ICQ2000::STATUS_NA:
      p = Icon_Status_NA.get();
      break;
    case ICQ2000::STATUS_OCCUPIED:
      p = Icon_Status_Occupied.get();
      break;
    case ICQ2000::STATUS_DND:
      p = Icon_Status_DND.get();
      break;
    case ICQ2000::STATUS_FREEFORCHAT:
      p = Icon_Status_FFC.get();
      break;
    case ICQ2000::STATUS_OFFLINE:
      p = Icon_Status_Offline.get();
      break;
    default:
      p = Icon_Status_Offline.get();
      break;
    }
  }
  
  return p;
}

Gtk::ImageLoader* Icons::IconForEvent(ICQMessageEvent::ICQMessageType t) {
  Gtk::ImageLoader *p;
  switch(t) {
  case ICQMessageEvent::Normal:
    p = Icon_Status_Message.get();
    break;
  case ICQMessageEvent::URL:
    p = Icon_Status_URL.get();
    break;
  case ICQMessageEvent::SMS:
  case ICQMessageEvent::SMS_Receipt:
    p = Icon_Status_SMS.get();
    break;
  case ICQMessageEvent::AuthReq:
  case ICQMessageEvent::AuthAck:
  case ICQMessageEvent::EmailEx:
  case ICQMessageEvent::WebPager:
  case ICQMessageEvent::UserAdd:
  default:
    p = Icon_Status_SystemMessage.get();
    break;
  }
  return p;
}
