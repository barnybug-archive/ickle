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

#include <gtk--/dialog.h>
#include <gtk--/button.h>
#include <gtk--/radiobutton.h>
#include <gtk--/combo.h>

#include <libicq2000/ContactTree.h>

namespace ICQ2000
{
  class ContactListEvent;
}

class GroupItem : public Gtk::ListItem 
{
 private:
  const ICQ2000::ContactTree::Group *m_libicq2000_group;
  
 public:
  GroupItem(const ICQ2000::ContactTree::Group *gp, gfloat x=0.0, gfloat y=0.5);

  const ICQ2000::ContactTree::Group * get_group();
};

class RemoveGroupDialog : public Gtk::Dialog {
 private:
  Gtk::Button m_ok, m_cancel;
  Gtk::RadioButton m_remove_all, m_move_all;
  Gtk::Combo m_group_list;
  ICQ2000::ContactTree::Group *m_libicq2000_group;

  void contactlist_cb(ICQ2000::ContactListEvent *ev);

 public:
  RemoveGroupDialog(Gtk::Window *parent, ICQ2000::ContactTree::Group *gp);

  void ok_cb();
};

#endif
