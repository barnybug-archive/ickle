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

#include "AddMobileUserDialog.h"

AddMobileUserDialog::AddMobileUserDialog()
  : Gtk::Dialog(),
    okay("OK"), cancel("Cancel")
{
  set_title("Add Mobile User");
  set_modal(true);

  okay.clicked.connect(slot(this,&AddMobileUserDialog::okay_cb));
  cancel.clicked.connect(slot(this,&AddMobileUserDialog::cancel_cb));

  Gtk::HBox *hbox = get_action_area();
  hbox->pack_start(okay, true, true, 0);
  hbox->pack_start(cancel, true, true, 0);

  Gtk::Table *table = manage( new Gtk::Table( 2, 2, false ) );

  Gtk::Label *label;
  label = manage( new Gtk::Label( "Alias", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  table->attach( alias_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Mobile No", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND, 10);
  table->attach( mobileno_entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND, 0);

  Gtk::VBox *vbox = get_vbox();
  vbox->pack_start( *table, true, true );

  set_border_width(10);
  set_usize(300,150);
  show_all();
}

bool AddMobileUserDialog::run() {
  Gtk::Main::run();
  if (finished_okay) {
    if (mobileno_entry.get_text().empty()) return false;
    return true;
  } else {
    return false;
  }
}

string AddMobileUserDialog::getAlias() const {
  return alias_entry.get_text();
}

string AddMobileUserDialog::getMobileNo() const {
  return mobileno_entry.get_text();
}

void AddMobileUserDialog::okay_cb() {
  Gtk::Main::quit();
  finished_okay = true;
}

void AddMobileUserDialog::cancel_cb() {
  Gtk::Main::quit();
  finished_okay = false;
}
