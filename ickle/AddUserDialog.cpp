/*
 * AddUserDialog
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

#include "AddUserDialog.h"

AddUserDialog::AddUserDialog()
  : Gtk::Dialog(),
    okay("OK"), cancel("Cancel")
{
  set_title("Add User");
  set_modal(true);

  okay.clicked.connect(slot(this,&AddUserDialog::okay_cb));
  cancel.clicked.connect(slot(this,&AddUserDialog::cancel_cb));

  Gtk::HBox *hbox = get_action_area();
  hbox->pack_start(okay, true, true, 0);
  hbox->pack_start(cancel, true, true, 0);

  Gtk::VBox *vbox = get_vbox();
  vbox->pack_start( *manage(new Gtk::Label("Enter User Number (UIN)")), true, true, 10 );
  vbox->pack_start( entry, true, true, 10 );

  set_border_width(10);
  set_usize(200,150);
  show_all();
}

unsigned int AddUserDialog::run() {
  Gtk::Main::run();
  return ICQ2000::Contact::StringtoUIN(entry.get_text());
}

void AddUserDialog::okay_cb() {
  Gtk::Main::quit();
}

void AddUserDialog::cancel_cb() {
  entry.set_text("");
  Gtk::Main::quit();
}

