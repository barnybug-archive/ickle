/*
 * SetAutoResponseDialog: Modeless dialog for changing auto response
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

#ifndef SETAWAYMSGDIALOG_H
#define SETAWAYMSGDIALOG_H

#include <gtk--/main.h>
#include <gtk--/dialog.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/text.h>
#include <gtk--/tooltips.h>
#include <string>
#include <sigc++/signal_system.h>

using SigC::Signal1;

class SetAutoResponseDialog : public Gtk::Dialog {
 private:
  Gtk::Button okay, cancel;
  Gtk::Text msg_input;
  Gtk::Tooltips m_tooltip;
  unsigned int m_timeout;
  SigC::Connection timeout_connection;

 public:
  SetAutoResponseDialog(const string& prev_msg);

  gint key_press_event_impl(GdkEventKey* ev);
  gint button_press_event_impl(GdkEventButton* ev);
  int auto_timeout();
  void cancel_timeout();

  void okay_cb();
  void cancel_cb();

  Signal1<void, const string&> save_new_msg;
};

#endif