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

#include <string>

#include "main.h"

#include "constants.h"
#include "events.h"

using Gtk::ImageLoader;
using Gtk::ImageLoaderData;
using ICQ2000::MessageEvent;
using std::string;

class Icons {
 private:
  static ImageLoader *Icon_Status_Online;
  static ImageLoader *Icon_Status_Away;
  static ImageLoader *Icon_Status_NA;
  static ImageLoader *Icon_Status_Occupied;
  static ImageLoader *Icon_Status_DND;
  static ImageLoader *Icon_Status_FFC;
  static ImageLoader *Icon_Status_Offline;
  static ImageLoader *Icon_Status_Message;
  static ImageLoader *Icon_Status_URL;
  static ImageLoader *Icon_Status_SMS;
  static ImageLoader *Icon_Status_Invisible;

 public:
  static void setDefaultIcons();
  static bool setIcons(const string& dir);
  static void FreeIcons();
  static ImageLoader* IconForStatus(Status s,bool inv);
  static ImageLoader* IconForEvent(MessageEvent::MessageType t);
};

#endif
