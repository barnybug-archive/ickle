/* $Id: Icons.h,v 1.13 2002-07-20 18:14:13 barnabygray Exp $
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

#include <gtk--/imageloader.h>
#include <sigc++/signal_system.h>

#include <memory>
#include <string>

#include "main.h"

#include <libicq2000/constants.h>

#include "MessageEvent.h"

class Icons {
 private:
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_Online;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_Away;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_NA;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_Occupied;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_DND;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_FFC;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_Offline;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_Message;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_URL;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_SMS;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_SystemMessage;
  std::auto_ptr<Gtk::ImageLoader> Icon_Status_Invisible;

 public:
  Icons();

  void setDefaultIcons();
  bool setIcons(const std::string& dir);
  Gtk::ImageLoader* IconForStatus(ICQ2000::Status s,bool inv);
  Gtk::ImageLoader* IconForEvent(ICQMessageEvent::ICQMessageType t);

  void settings_changed_cb(const std::string& key);

  SigC::Signal0<void> icons_changed;

};

#endif
