/* $Id: Icons.h,v 1.10 2002-01-16 12:58:40 barnabygray Exp $
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
#include <libicq2000/events.h>

using Gtk::ImageLoader;
using Gtk::ImageLoaderData;
using ICQ2000::MessageEvent;
using std::string;

class Icons {
 private:
  std::auto_ptr<ImageLoader> Icon_Status_Online;
  std::auto_ptr<ImageLoader> Icon_Status_Away;
  std::auto_ptr<ImageLoader> Icon_Status_NA;
  std::auto_ptr<ImageLoader> Icon_Status_Occupied;
  std::auto_ptr<ImageLoader> Icon_Status_DND;
  std::auto_ptr<ImageLoader> Icon_Status_FFC;
  std::auto_ptr<ImageLoader> Icon_Status_Offline;
  std::auto_ptr<ImageLoader> Icon_Status_Message;
  std::auto_ptr<ImageLoader> Icon_Status_URL;
  std::auto_ptr<ImageLoader> Icon_Status_SMS;
  std::auto_ptr<ImageLoader> Icon_Status_Invisible;

 public:
  Icons();

  void setDefaultIcons();
  bool setIcons(const string& dir);
  ImageLoader* IconForStatus(ICQ2000::Status s,bool inv);
  ImageLoader* IconForEvent(MessageEvent::MessageType t);

  void settings_changed_cb(const string& key);

  SigC::Signal0<void> icons_changed;

};

#endif
