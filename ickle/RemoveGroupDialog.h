/*
 * RemoveGroupDialog
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

#ifndef REMOVEGROUPDIALOG_H
#define REMOVEGROUPDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/combo.h>

#include <libicq2000/ContactTree.h>

#include <map>

namespace ICQ2000
{
  class ContactListEvent;
}

class RemoveGroupDialog : public Gtk::Dialog,
			  public sigslot::has_slots<>
{
 private:
  ICQ2000::ContactTree::Group *m_libicq2000_group;
  ICQ2000::ContactTree::Group *m_selected_group;
  
  Gtk::Button m_ok, m_cancel;
  Gtk::RadioButton m_remove_all, m_move_all;
  Gtk::Combo m_group_list;
  std::map< unsigned int, Gtk::ComboDropDownItem * > m_group_map;

  void on_response(int response_id);

  void contactlist_cb(ICQ2000::ContactListEvent *ev);
  void selected_group_cb(ICQ2000::ContactTree::Group *gp);

 public:
  RemoveGroupDialog(Gtk::Window& parent, ICQ2000::ContactTree::Group *gp);
};

#endif
