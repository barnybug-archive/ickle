/*
 * SetEncodingDialog
 * Copyright (C) 2003 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#ifndef SETENCODINGDIALOG_H
#define SETENCODINGDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

#include <libicq2000/Contact.h>

namespace ICQ2000
{
  class ContactListEvent;
}

class SetEncodingDialog : public Gtk::Dialog,
			  public sigslot::has_slots<>
{
 private:
  ICQ2000::ContactRef m_contact;
  Gtk::Entry m_encoding;
  Gtk::Label m_valid;

 protected:
  virtual void on_response(int response_id);
  void validate_encoding_cb();
  void contactlist_cb(ICQ2000::ContactListEvent *ev);

 public:
  SetEncodingDialog(Gtk::Window& parent, const ICQ2000::ContactRef& c);
};

#endif
