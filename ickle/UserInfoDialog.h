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
#include <gtk--/notebook.h>
#include <gtk--/text.h>
#include <gtk--/combo.h>
#include <gtk--/spinbutton.h>

#include <sigc++/signal_system.h>

#include <libicq2000/Contact.h>

using ICQ2000::Contact;
using ICQ2000::MainHomeInfo;
using ICQ2000::HomepageInfo;

using SigC::Signal0;

class UserInfoDialog : public Gtk::Dialog {
 private:
  Gtk::Button okay, cancel, fetchb;
  Gtk::Entry uin_entry, alias_entry, firstname_entry, lastname_entry,
    email_entry1, email_entry2, email_entry3, ip_entry, status_entry, timezone_entry,
    addr_entry, phone_entry, state_entry, fax_entry, city_entry, cellular_entry,
    zip_entry, country_entry, age_entry, sex_entry, homepage_entry, birthday_entry,
    lang_entry1, lang_entry2, lang_entry3;

  Gtk::SpinButton birth_year_spin, birth_month_spin, birth_day_spin;
  Gtk::Combo country_combo, lang_combo1, lang_combo2, lang_combo3, sex_combo;
  Gtk::Text about_text;
  Gtk::Notebook notebook;

  Contact *contact;
  bool changed;
  bool m_self;

  bool update_contact();

 public:
  UserInfoDialog(Contact *c, bool self = false);
  ~UserInfoDialog();

  // -- gui callbacks --
  void okay_cb();
  void upload_cb();

  // -- library callbacks --
  void userinfochange_cb();

  void raise() const;

  bool isChanged() const;

  Signal0<void> fetch;
  Signal0<void> upload;
};

#endif
