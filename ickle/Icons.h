/* $Id: Icons.h,v 1.18 2003-05-26 15:52:27 barnabygray Exp $
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

#ifndef ICONS_H
#define ICONS_H

#include <gdkmm/pixbuf.h>
#include <sigc++/signal.h>

#include <string>
#include <vector>

#include "main.h"

#include <libicq2000/constants.h>

#include "MessageEvent.h"

class Icons {
 private:
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_Online;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_Away;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_NA;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_Occupied;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_DND;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_FFC;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_Offline;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_Message;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_URL;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_SMS;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_FileTransfer;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_SystemMessage;
  Glib::RefPtr<Gdk::Pixbuf> Icon_Status_Invisible;

  void create_from_file_no_exceptions( Glib::RefPtr<Gdk::Pixbuf>& refptr, const std::string& file );

 public:
  Icons();

  void setDefaultIcons();
  std::vector<std::string> get_icon_sets();
  bool setIcons(const std::string& dir);
  Glib::RefPtr<Gdk::Pixbuf> get_icon_for_status(ICQ2000::Status s,bool inv);
  Glib::RefPtr<Gdk::Pixbuf> get_icon_for_status(ICQ2000::Status s, const std::string& dir, bool inv);
  Glib::RefPtr<Gdk::Pixbuf> get_icon_for_event(ICQMessageEvent::ICQMessageType t);

  void settings_changed_cb(const std::string& key);

  SigC::Signal0<void> icons_changed;

};

#endif
