/*
 * FindTextDialog
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 * Copyright (C) 2003 Christian Borntraeger <linux@borntraeger.net>.
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

#ifndef FINDTEXTDIALOG_H
#define FINDTEXTDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/tooltips.h>

#include <sigc++/sigc++.h>

class FindTextDialog : public Gtk::Dialog
{
 private:
  Gtk::Entry m_search_text;
  Gtk::Tooltips m_tooltip;
  Gtk::CheckButton m_case_sensitive;
  enum
  {
    RESPONSE_FIND=1
  };

 protected:
  virtual void on_response(int response_id);
  void change_cb();

 public:
  FindTextDialog(Gtk::Window& parent, const Glib::ustring title, const Glib::ustring question,
		 const Glib::ustring oldsearch);
  ~FindTextDialog();

  SigC::Signal2<void,Glib::ustring,bool> signal_textsubmit;
};

#endif
