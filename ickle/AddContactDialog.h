/*
 * AddContactDialog
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

#ifndef ADDCONTACTDIALOG_H
#define ADDCONTACTDIALOG_H

#include <map>

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/frame.h>
#include <gtkmm/combo.h>

#include "MobileNoEntry.h"

#include <libicq2000/Contact.h>
#include <libicq2000/ContactTree.h>

namespace ICQ2000
{
  class ContactListEvent;
}

class AddContactDialog : public Gtk::Dialog,
			 public sigslot::has_slots<>
{
 private:
  Gtk::RadioButton m_icq_contact, m_mobile_contact;
  Gtk::Label m_uin_label;
  Gtk::Entry m_uin_entry;
  Gtk::CheckButton m_alert_check;
  Gtk::Frame m_mode_frame, m_icq_frame, m_mobile_frame, m_group_frame;

  Gtk::Label m_alias_label, m_mobileno_label;
  Gtk::Entry m_alias_entry;
  MobileNoEntry m_mobileno_entry;
  Gtk::Combo m_group_list;
  std::map< unsigned int, Gtk::ComboDropDownItem * > m_group_map;
  ICQ2000::ContactTree::Group * m_selected_group;
  
  Gtk::Button& m_ok_button;


  void update_stuff();
  void uin_changed_cb();
  void mobileno_changed_cb();

  void on_response(int response_id);

  void contactlist_cb(ICQ2000::ContactListEvent * ev);
  void selected_group_cb(ICQ2000::ContactTree::Group * gp);

 public:
  AddContactDialog(Gtk::Window& parent);
  static ICQ2000::ContactTree::Group * create_new_group();
};

#endif
