/* $Id: Icons.cpp,v 1.7 2001-12-18 19:45:10 nordman Exp $
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

/*
  Sets the icons to the default compiled in icons.

  Note: This also automatically updates the settings for the icons.
*/
void Icons::setDefaultIcons() {
  Icon_Status_Online = new ImageLoaderData(online_xpm);
  Icon_Status_Away = new ImageLoaderData(away_xpm);
  Icon_Status_NA = new ImageLoaderData(na_xpm);
  Icon_Status_Occupied = new ImageLoaderData(occ_xpm);
  Icon_Status_DND = new ImageLoaderData(dnd_xpm);
  Icon_Status_FFC = new ImageLoaderData(ffc_xpm);
  Icon_Status_Offline = new ImageLoaderData(offline_xpm);
  Icon_Status_Message = new ImageLoaderData(message_xpm);
  Icon_Status_URL = new ImageLoaderData(url_xpm);
  Icon_Status_SMS = new ImageLoaderData(sms_xpm);
  Icon_Status_Invisible = new ImageLoaderData(invisible_xpm);
  g_settings.setValue("icons_dir", "Default");
}

/*
  Sets the icons to the set given by dir.

  Note: This also automatically updates the settings for the icons.
*/
bool Icons::setIcons(const string &dir) {
  FreeIcons();

  if (dir == "" || dir == "Default") {
    setDefaultIcons();
    return true;
  }

  Icon_Status_Online = new ImageLoader( dir + "online.xpm" );
  Icon_Status_Away = new ImageLoader( dir + "away.xpm" );
  Icon_Status_NA = new ImageLoader( dir + "na.xpm" );
  Icon_Status_Occupied = new ImageLoader( dir + "occ.xpm" );
  Icon_Status_DND = new ImageLoader( dir + "dnd.xpm" );
  Icon_Status_FFC = new ImageLoader( dir + "ffc.xpm" );
  Icon_Status_Offline = new ImageLoader( dir + "offline.xpm" );
  Icon_Status_Message = new ImageLoader( dir + "message.xpm" );
  Icon_Status_URL = new ImageLoader( dir + "url.xpm" );
  Icon_Status_SMS = new ImageLoader( dir + "sms.xpm" );
  Icon_Status_Invisible = new ImageLoader( dir + "invisible.xpm" );
  g_settings.setValue("icons_dir", dir);
  return true;
}

void Icons::FreeIcons() {
  delete Icon_Status_Online;
  delete Icon_Status_Away;
  delete Icon_Status_NA;
  delete Icon_Status_Occupied;
  delete Icon_Status_DND;
  delete Icon_Status_FFC;
  delete Icon_Status_Offline;
  delete Icon_Status_Message;
  delete Icon_Status_URL;
  delete Icon_Status_SMS;
  delete Icon_Status_Invisible;
}

ImageLoader* Icons::IconForStatus(Status s,bool inv) { 
  ImageLoader *p;
  switch(s) {
  case STATUS_ONLINE:
    if (inv) {
    p = Icon_Status_Invisible;
    } else {
      p = Icon_Status_Online;
    } 
    break;
  case STATUS_AWAY:
    p = Icon_Status_Away;
    break;
  case STATUS_NA:
    p = Icon_Status_NA;
    break;
  case STATUS_OCCUPIED:
    p = Icon_Status_Occupied;
    break;
  case STATUS_DND:
    p = Icon_Status_DND;
    break;
  case STATUS_FREEFORCHAT:
    p = Icon_Status_FFC;
    break;
  case STATUS_OFFLINE:
    p = Icon_Status_Offline;
    break;
  default:
    p = Icon_Status_Offline;
    break;
  }
  return p;
}

ImageLoader* Icons::IconForEvent(MessageEvent::MessageType t) {
  ImageLoader *p;
  switch(t) {
  case MessageEvent::Normal:
    p = Icon_Status_Message;
    break;
  case MessageEvent::URL:
    p = Icon_Status_URL;
    break;
  case MessageEvent::SMS:
  case MessageEvent::SMS_Receipt:
  default:
    p = Icon_Status_SMS;
    break;
  }
  return p;
}
