/*
 * AddGroupDialog
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

#include "AddGroupDialog.h"

// Client object
#include "main.h"
#include <libicq2000/Client.h>

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>

AddGroupDialog::AddGroupDialog(Gtk::Window& parent)
  : Gtk::Dialog("Add Group", parent)
{
  set_position(Gtk::WIN_POS_CENTER);

  Gtk::Label *label;
  
  add_button(Gtk::Stock::ADD,    Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  Gtk::HBox *hbox2 = manage( new Gtk::HBox() );
  hbox2->set_spacing(5);
  label = manage( new Gtk::Label("Group label") );
  hbox2->pack_start( *label );
  hbox2->pack_start( m_group_label );
  vbox->pack_start( *hbox2 );

  set_border_width(10);
  show_all();
}

void AddGroupDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    ICQ2000::ContactTree& ct = icqclient.getContactTree();
    ct.add_group( m_group_label.get_text() );
  }

  delete this;
}
