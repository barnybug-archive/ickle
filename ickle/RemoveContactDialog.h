/*
 * RemoveContactDialog
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#ifndef REMOVECONTACTDIALOG_H
#define REMOVECONTACTDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/frame.h>

#include "MobileNoEntry.h"

#include <libicq2000/Contact.h>

namespace ICQ2000
{
  class ContactListEvent;
}

class RemoveContactDialog : public Gtk::Dialog,
			    public sigslot::has_slots<>
{
 private:
  ICQ2000::ContactRef m_contact;

  void contactlist_cb(ICQ2000::ContactListEvent *ev);

  void on_response(int response_id);

 public:
  RemoveContactDialog(Gtk::Window& parent, const ICQ2000::ContactRef& c);
};

#endif
