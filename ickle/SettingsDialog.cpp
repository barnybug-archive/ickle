/*
 * SettingsDialog
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

#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(Settings& settings)
  : Gtk::Dialog(),
    okay("OK"), cancel("Cancel")
{
  set_title("Settings Dialog");
  set_modal(true);

  okay.clicked.connect(slot(this,&SettingsDialog::okay_cb));
  cancel.clicked.connect(slot(this,&SettingsDialog::cancel_cb));

  notebook.set_tab_pos(GTK_POS_TOP);

  Gtk::HBox *hbox = get_action_area();
  hbox->pack_start(okay, true, true, 0);
  hbox->pack_start(cancel, true, true, 0);

  // ---------------- Login Details tab -------------------------

  Gtk::Table *table = manage( new Gtk::Table( 3, 2, false ) );

  Gtk::Label *label;
  label = manage( new Gtk::Label( "UIN", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, 0);
  uin_entry.set_text(ICQ2000::Contact::UINtoString( settings.getValueUnsignedInt("uin") ));
  table->attach( uin_entry, 1, 3, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "Password", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, 0);
  password_entry.set_text( settings.getValueString("password") );
  password_entry.set_visibility(false);
  table->attach( password_entry, 1, 3, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "Login details" ) );
  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *table, *label )  );
  // ------------------------------------------------------------

  // ---------------- Events tab --------------------------
  table = manage( new Gtk::Table( 2, 4, false ) );
  
  label = manage( new Gtk::Label( "Below you can enter in commands to be executed when you receive an event. "
				  "Leave them blank if you don't want anything to happen. "
				  "They will be executed by /bin/sh, so echo -e \"\\a\" will work.", 0 ) );
  label->set_line_wrap(true);
  table->attach( *label, 0, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  label = manage( new Gtk::Label( "Message Event", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, 0);
  event_message_entry.set_text( settings.getValueString("event_message") );
  table->attach( event_message_entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "URL Event", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND, 0);
  event_url_entry.set_text( settings.getValueString("event_url") );
  table->attach( event_url_entry, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "SMS Event", 0 ) );
  table->attach( *label, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND, 0);
  event_sms_entry.set_text( settings.getValueString("event_sms") );
  table->attach( event_sms_entry, 1, 2, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "Events" ) );
  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *table, *label )  );

  Gtk::VBox *vbox = get_vbox();
  vbox->pack_start( notebook, true, true );


  set_border_width(10);
  set_usize(500, 300);
  show_all();
}

bool SettingsDialog::run() {
  Gtk::Main::run();
  if (finished_okay) {
    return true;
  } else {
    return false;
  }
}

void SettingsDialog::updateSettings(Settings& settings) {
  // ------------ Login details tab ----------------
  settings.setValue("uin", ICQ2000::Contact::StringtoUIN(uin_entry.get_text()));
  settings.setValue("password", password_entry.get_text());

  // ------------ Events tab -----------------------
  settings.setValue("event_message", event_message_entry.get_text());
  settings.setValue("event_url", event_url_entry.get_text());
  settings.setValue("event_sms", event_sms_entry.get_text());
}

unsigned int SettingsDialog::getUIN() const {
  return ICQ2000::Contact::StringtoUIN(uin_entry.get_text());
}

string SettingsDialog::getPassword() const {
  return password_entry.get_text();
}

void SettingsDialog::okay_cb() {
  Gtk::Main::quit();
  finished_okay = true;
}

void SettingsDialog::cancel_cb() {
  Gtk::Main::quit();
  finished_okay = false;
}
