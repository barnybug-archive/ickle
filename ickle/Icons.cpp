/* $Id: Icons.cpp,v 1.24 2003-05-26 15:52:27 barnabygray Exp $
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
#include "file.xpm"
#include "sysmsg.xpm"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

using std::string;

using ICQ2000::Status;

Icons::Icons()
{
}

/*
 * Sets the icons to the default compiled in icons.
 */
void Icons::setDefaultIcons()
{
  Icon_Status_Online = Gdk::Pixbuf::create_from_xpm_data( (online_xpm) );
  Icon_Status_Away = Gdk::Pixbuf::create_from_xpm_data( (away_xpm) );
  Icon_Status_NA = Gdk::Pixbuf::create_from_xpm_data( (na_xpm) );
  Icon_Status_Occupied = Gdk::Pixbuf::create_from_xpm_data( (occ_xpm) );
  Icon_Status_DND = Gdk::Pixbuf::create_from_xpm_data( (dnd_xpm) );
  Icon_Status_FFC = Gdk::Pixbuf::create_from_xpm_data( (ffc_xpm) );
  Icon_Status_Offline = Gdk::Pixbuf::create_from_xpm_data( (offline_xpm) );
  Icon_Status_Message = Gdk::Pixbuf::create_from_xpm_data( (message_xpm) );
  Icon_Status_URL = Gdk::Pixbuf::create_from_xpm_data( (url_xpm) );
  Icon_Status_SMS = Gdk::Pixbuf::create_from_xpm_data( (sms_xpm) );
  Icon_Status_FileTransfer = Gdk::Pixbuf::create_from_xpm_data( (file_xpm) );
  Icon_Status_SystemMessage = Gdk::Pixbuf::create_from_xpm_data( (sysmsg_xpm) );
  Icon_Status_Invisible = Gdk::Pixbuf::create_from_xpm_data( (invisible_xpm) );
}

void Icons::settings_changed_cb(const string& key)
{
  if (key == "icons_dir") {
    setIcons( g_settings.getValueString("icons_dir") );
  }
}

bool Icons::setIcons(const string &dir)
{
  setDefaultIcons();

  if (dir == "" || dir == "Default")
  {
    icons_changed.emit();
    return true;
  }

  /* We don't want missing icon files to cause the whole lot to fail,
   * so quietly trap the exceptions individually */
  create_from_file_no_exceptions( Icon_Status_Online,        dir + "online.xpm" );
  create_from_file_no_exceptions( Icon_Status_Away,          dir + "away.xpm" );
  create_from_file_no_exceptions( Icon_Status_NA,            dir + "na.xpm" );
  create_from_file_no_exceptions( Icon_Status_Occupied,      dir + "occ.xpm" );
  create_from_file_no_exceptions( Icon_Status_DND,           dir + "dnd.xpm" );
  create_from_file_no_exceptions( Icon_Status_FFC,           dir + "ffc.xpm" );
  create_from_file_no_exceptions( Icon_Status_Offline,       dir + "offline.xpm" );
  create_from_file_no_exceptions( Icon_Status_Message,       dir + "message.xpm" );
  create_from_file_no_exceptions( Icon_Status_URL,           dir + "url.xpm" );
  create_from_file_no_exceptions( Icon_Status_SMS,           dir + "sms.xpm" );
  create_from_file_no_exceptions( Icon_Status_FileTransfer,  dir + "file.xpm" );
  create_from_file_no_exceptions( Icon_Status_SystemMessage, dir + "sysmsg.xpm" );
  create_from_file_no_exceptions( Icon_Status_Invisible,     dir + "invisible.xpm" );
  
  icons_changed.emit();

  return true;
}

void Icons::create_from_file_no_exceptions( Glib::RefPtr<Gdk::Pixbuf>& refptr, const std::string& file )
{
  try
  {
    refptr = Gdk::Pixbuf::create_from_file(file);
  }
  catch(Glib::FileError)
  {
  }
}

Glib::RefPtr<Gdk::Pixbuf> Icons::get_icon_for_status(Status s, bool inv)
{
  Glib::RefPtr<Gdk::Pixbuf> ret;

  if (inv && s != ICQ2000::STATUS_OFFLINE)
  {
    ret = Icon_Status_Invisible;
  }
  else
  {
    switch(s)
    {
    case ICQ2000::STATUS_ONLINE:
      ret = Icon_Status_Online;
      break;
    case ICQ2000::STATUS_AWAY:
      ret = Icon_Status_Away;
      break;
    case ICQ2000::STATUS_NA:
      ret = Icon_Status_NA;
      break;
    case ICQ2000::STATUS_OCCUPIED:
      ret = Icon_Status_Occupied;
      break;
    case ICQ2000::STATUS_DND:
      ret = Icon_Status_DND;
      break;
    case ICQ2000::STATUS_FREEFORCHAT:
      ret = Icon_Status_FFC;
      break;
    case ICQ2000::STATUS_OFFLINE:
      ret = Icon_Status_Offline;
      break;
    default:
      ret = Icon_Status_Offline;
      break;
    }
  }

  return ret;
}


Glib::RefPtr<Gdk::Pixbuf> Icons::get_icon_for_status(Status s, const std::string& dir, bool inv)
{
  Glib::RefPtr<Gdk::Pixbuf> ret;

  if (inv && s != ICQ2000::STATUS_OFFLINE)
  {
    return Gdk::Pixbuf::create_from_file(dir + "invisible.xpm");
  }
  else
  {
    switch(s)
    {
      case ICQ2000::STATUS_ONLINE:
        return Gdk::Pixbuf::create_from_file(dir + "online.xpm");
      case ICQ2000::STATUS_AWAY:
        return Gdk::Pixbuf::create_from_file(dir + "away.xpm");
      case ICQ2000::STATUS_NA:
        return Gdk::Pixbuf::create_from_file(dir + "na.xpm");
      case ICQ2000::STATUS_OCCUPIED:
        return Gdk::Pixbuf::create_from_file(dir + "occ.xpm");
      case ICQ2000::STATUS_DND:
        return Gdk::Pixbuf::create_from_file(dir + "dnd.xpm");
      case ICQ2000::STATUS_FREEFORCHAT:
        return Gdk::Pixbuf::create_from_file(dir + "ffc.xpm");
      case ICQ2000::STATUS_OFFLINE:
        return Gdk::Pixbuf::create_from_file(dir + "offline.xpm");
      default:
        return Gdk::Pixbuf::create_from_file(dir + "offline.xpm");
    }
  }
 }


Glib::RefPtr<Gdk::Pixbuf> Icons::get_icon_for_event(ICQMessageEvent::ICQMessageType t)
{
  Glib::RefPtr<Gdk::Pixbuf> ret;

  switch(t) {
  case ICQMessageEvent::Normal:
    ret = Icon_Status_Message;
    break;
  case ICQMessageEvent::URL:
    ret = Icon_Status_URL;
    break;
  case ICQMessageEvent::FileTransfer:
    ret = Icon_Status_FileTransfer;
    break;
  case ICQMessageEvent::SMS:
  case ICQMessageEvent::SMS_Receipt:
    ret = Icon_Status_SMS;
    break;
  case ICQMessageEvent::AuthReq:
  case ICQMessageEvent::AuthAck:
  case ICQMessageEvent::EmailEx:
  case ICQMessageEvent::WebPager:
  case ICQMessageEvent::UserAdd:
  default:
    ret = Icon_Status_SystemMessage;
    break;
  }
  return ret;
}


std::vector<std::string> Icons::get_icon_sets()
{
  std::vector<std::string> buf;
  DIR *pDir;
  
  pDir = opendir(ICONS_DIR.c_str());
  if (pDir)
  {
    struct dirent *rdp;

    while ( (rdp = readdir(pDir)) != NULL)
    {
      if (strcmp(rdp->d_name, ".") == 0 || strcmp(rdp->d_name, "..") == 0)
	continue;

      std::string fullname = ICONS_DIR + "/" + rdp->d_name;

      struct stat st;
      if ( stat(fullname.c_str(), &st) == 0 && S_ISDIR(st.st_mode) )
	buf.push_back( fullname );
    }

    closedir(pDir);
  }
  
  return buf;
}

