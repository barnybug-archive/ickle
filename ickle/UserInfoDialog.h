/*
 * UserInfoDialog
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

#ifndef USERINFODIALOG_H
#define USERINFODIALOG_H

#include <gtk--/main.h>
#include <gtk--/dialog.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/entry.h>
#include <gtk--/table.h>

#include <sstream>

#include "Contact.h"

using ICQ2000::Contact;

class UserInfoDialog : public Gtk::Dialog {
 private:
  Gtk::Button okay, cancel;
  Gtk::Entry uin_entry, alias_entry, mobileno_entry;

  Contact *contact;
  bool finished_okay;

 public:
  UserInfoDialog(Contact *c);

  bool run();

  void okay_cb();
  void cancel_cb();
};

#endif
