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

#include "UserInfoDialog.h"

UserInfoDialog::UserInfoDialog(Contact *c)
       : Gtk::Dialog(),
	 okay("OK"), cancel("Cancel"), fetchb("Fetch"),
	 contact(c)
{
  ostringstream ostr;
  ostr << "User Info - " << c->getAlias() << " (";
  if (c->isICQContact()) {
    ostr << c->getUIN();
  } else {
    ostr << c->getMobileNo();
  }
  ostr << ")";
  set_title(ostr.str());
  set_modal(true);

  okay.clicked.connect(slot(this,&UserInfoDialog::okay_cb));
  cancel.clicked.connect(slot(this,&UserInfoDialog::cancel_cb));
  fetchb.clicked.connect( fetch.slot() );

  Gtk::Label *label;

  Gtk::HBox *hbox = get_action_area();
  hbox->pack_start(fetchb, true, true, 0);
  hbox->pack_start(okay, true, true, 0);
  hbox->pack_start(cancel, true, true, 0);

  Gtk::Table *table = manage( new Gtk::Table( 3, 3, false ) );

  label = manage( new Gtk::Label( "UIN", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  if (c->isICQContact()) uin_entry.set_text( ICQ2000::Contact::UINtoString(c->getUIN()) );
  uin_entry.set_editable(false);
  uin_entry.set_sensitive(false);
  table->attach( uin_entry, 1, 3, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Alias", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND, 10);
  alias_entry.set_text( c->getAlias() );
  table->attach( alias_entry, 1, 3, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Mobile No", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND, 10);
  label = manage( new Gtk::Label( "+", 0) );
  table->attach( *label, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 3);
  mobileno_entry.set_text( c->getMobileNo() );
  table->attach( mobileno_entry, 2, 3, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  Gtk::VBox *vbox = get_vbox();
  vbox->pack_start( *table, true, true );

  set_border_width(10);
  set_usize(270,150);
  show_all();
}

bool UserInfoDialog::run() {
  Gtk::Main::run();
  if (finished_okay) {
    // check if anything was touched
    finished_okay = false;
    if (contact->getAlias() != alias_entry.get_text()) {
      finished_okay = true;
      contact->setAlias(alias_entry.get_text());
    }
    if (contact->getMobileNo() != mobileno_entry.get_text()) {
      finished_okay = true;
      contact->setMobileNo(mobileno_entry.get_text());
    }
    return finished_okay;
  }
}

void UserInfoDialog::okay_cb() {
  finished_okay = true;
  Gtk::Main::quit();
}

void UserInfoDialog::cancel_cb() {
  finished_okay = false;
  Gtk::Main::quit();
}

void UserInfoDialog::userinfochange_cb() {
  alias_entry.set_text( contact->getAlias() );
  mobileno_entry.set_text( contact->getMobileNo() );
}

