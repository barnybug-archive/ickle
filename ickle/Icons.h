/*
 * Icons
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

#include <string>

#include "main.h"

#include "constants.h"
#include "events.h"

using Gtk::ImageLoader;
using Gtk::ImageLoaderData;
using ICQ2000::MessageEvent;
using std::string;
using SigC::Signal0;

class Icons {
 private:
  ImageLoader *Icon_Status_Online;
  ImageLoader *Icon_Status_Away;
  ImageLoader *Icon_Status_NA;
  ImageLoader *Icon_Status_Occupied;
  ImageLoader *Icon_Status_DND;
  ImageLoader *Icon_Status_FFC;
  ImageLoader *Icon_Status_Offline;
  ImageLoader *Icon_Status_Message;
  ImageLoader *Icon_Status_URL;
  ImageLoader *Icon_Status_SMS;
  ImageLoader *Icon_Status_Invisible;

 public:
  void setDefaultIcons();
  bool setIcons(const string& dir);
  void FreeIcons();
  ImageLoader* IconForStatus(Status s,bool inv);
  ImageLoader* IconForEvent(MessageEvent::MessageType t);

  Signal0<void> icons_changed;

};

#endif
