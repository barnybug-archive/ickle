/*
 * AddMobileUserDialog
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

#ifndef ADDMOBILEUSERDIALOG_H
#define ADDMOBILEUSERDIALOG_H

#include <gtk--/main.h>
#include <gtk--/dialog.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/entry.h>
#include <gtk--/table.h>

#include "MobileNoEntry.h"

#include <string>

using namespace std;

class AddMobileUserDialog : public Gtk::Dialog {
 private:
  Gtk::Button okay, cancel;
  Gtk::Entry alias_entry;
  MobileNoEntry mobileno_entry;

 public:
  AddMobileUserDialog();

  void okay_cb();
  void cancel_cb();

  SigC::Signal2<void, std::string, std::string> add_mobile_user;
};

#endif
