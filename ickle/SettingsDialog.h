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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <gtk--/main.h>
#include <gtk--/dialog.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/entry.h>
#include <gtk--/notebook.h>
#include <gtk--/fileselection.h>
#include <gtk--/list.h>

#include <sigc++/signal_system.h>

#include <string>

#include "Settings.h"
#include "main.h"

#include "Contact.h"

using SigC::Signal1;
using std::string;

class SettingsDialog : public Gtk::Dialog {
 private:
  Gtk::Button okay, cancel, trans_b;
  Gtk::Label trans_l;
  Gtk::List icons_list;
  Gtk::Entry uin_entry, password_entry, event_message_entry, event_url_entry, event_sms_entry;
  Gtk::Notebook notebook;

  bool finished_okay;

  string getIconsFilename();
  
  void okay_cb();
  void cancel_cb();
  void trans_cb();
  void icons_cb();
  void trans_ok_cb(Gtk::FileSelection *fs);

 public:
  SettingsDialog();

  bool run();

  unsigned int getUIN() const;
  string getPassword() const;

  void updateSettings();

  Signal1<void,string> icons_changed;
};

#endif
