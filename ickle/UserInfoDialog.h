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

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/table.h>
#include <gtkmm/notebook.h>
#include <gtkmm/textview.h>
#include <gtkmm/combo.h>
#include <gtkmm/spinbutton.h>

#include <sigc++/signal.h>

#include <libicq2000/Contact.h>
#include <libicq2000/events.h>

class UserInfoDialog : public Gtk::Dialog
{
 private:
  SigC::Signal0<void> m_signal_destroy;
  SigC::Signal0<void> m_signal_fetch;
  SigC::Signal0<void> m_signal_upload;

  bool m_self;

 public:
  UserInfoDialog(Gtk::Window& parent, const ICQ2000::ContactRef& c, bool self);
  ~UserInfoDialog();

  SigC::Signal0<void>& signal_destroy();
  SigC::Signal0<void>& signal_fetch();
  SigC::Signal0<void>& signal_upload();
};

#endif
