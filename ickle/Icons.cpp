/* $Id: Icons.cpp,v 1.11 2002-01-16 12:58:40 barnabygray Exp $
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
#include "chat.xpm"
#include "invisible.xpm"
#include "message.xpm"
#include "url.xpm"
#include "sms.xpm"
#include "file.xpm"


using ICQ2000::Status;

Icons::Icons()
{
}

/*
 * Sets the icons to the default compiled in icons.
 */
void Icons::setDefaultIcons() {
  Icon_Status_Online.reset( new ImageLoaderData(online_xpm) );
  Icon_Status_Away.reset( new ImageLoaderData(away_xpm) );
  Icon_Status_NA.reset( new ImageLoaderData(na_xpm) );
  Icon_Status_Occupied.reset( new ImageLoaderData(occ_xpm) );
  Icon_Status_DND.reset( new ImageLoaderData(dnd_xpm) );
  Icon_Status_FFC.reset( new ImageLoaderData(ffc_xpm) );
  Icon_Status_Offline.reset( new ImageLoaderData(offline_xpm) );
  Icon_Status_Message.reset( new ImageLoaderData(message_xpm) );
  Icon_Status_URL.reset( new ImageLoaderData(url_xpm) );
  Icon_Status_SMS.reset( new ImageLoaderData(sms_xpm) );
  Icon_Status_Invisible.reset( new ImageLoaderData(invisible_xpm) );
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

  Icon_Status_Online.reset( new ImageLoader( dir + "online.xpm" ) );
  Icon_Status_Away.reset( new ImageLoader( dir + "away.xpm" ) );
  Icon_Status_NA.reset( new ImageLoader( dir + "na.xpm" ) );
  Icon_Status_Occupied.reset( new ImageLoader( dir + "occ.xpm" ) );
  Icon_Status_DND.reset( new ImageLoader( dir + "dnd.xpm" ) );
  Icon_Status_FFC.reset( new ImageLoader( dir + "ffc.xpm" ) );
  Icon_Status_Offline.reset( new ImageLoader( dir + "offline.xpm" ) );
  Icon_Status_Message.reset( new ImageLoader( dir + "message.xpm" ) );
  Icon_Status_URL.reset( new ImageLoader( dir + "url.xpm" ) );
  Icon_Status_SMS.reset( new ImageLoader( dir + "sms.xpm" ) );
  Icon_Status_Invisible.reset( new ImageLoader( dir + "invisible.xpm" ) );
  icons_changed.emit();
  return true;
}

ImageLoader* Icons::IconForStatus(Status s, bool inv) { 
  ImageLoader *p;
  if (inv) {
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

ImageLoader* Icons::IconForEvent(MessageEvent::MessageType t) {
  ImageLoader *p;
  switch(t) {
  case MessageEvent::Normal:
    p = Icon_Status_Message.get();
    break;
  case MessageEvent::URL:
    p = Icon_Status_URL.get();
    break;
  case MessageEvent::SMS:
  case MessageEvent::SMS_Receipt:
  default:
    p = Icon_Status_SMS.get();
    break;
  }
  return p;
}
