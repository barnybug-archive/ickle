/*
 * RenameGroupDialog
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

#include "RenameGroupDialog.h"

#include "main.h"
#include <libicq2000/Client.h>

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>

RenameGroupDialog::RenameGroupDialog(Gtk::Window& parent, ICQ2000::ContactTree::Group *gp)
  : Gtk::Dialog("Rename Group", parent), m_libicq2000_group( gp )
{
  set_position(Gtk::WIN_POS_CENTER);

  Gtk::Label *label;

  add_button("Rename",           Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  // libicq2000 callbacks
  icqclient.contactlist.connect( this, &RenameGroupDialog::contactlist_cb );

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  Gtk::Table *table = manage( new Gtk::Table(2, 2) );
  table->set_spacings(5);

  label = manage( new Gtk::Label("Old group label", 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  m_old_group_label.set_text( gp->get_label() );
  m_old_group_label.set_sensitive(false);
  m_old_group_label.set_editable(false);
  table->attach( m_old_group_label, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );

  label = manage( new Gtk::Label("New group label", 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  table->attach( m_group_label, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );

  vbox->pack_start( *table );

  set_border_width(10);
  show_all();
  m_group_label.grab_focus();
}

void RenameGroupDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    m_libicq2000_group->set_label( m_group_label.get_text() );
  }
  
  delete this;
}

void RenameGroupDialog::contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  if (ev->getType() == ICQ2000::ContactListEvent::GroupRemoved)
  {
    ICQ2000::GroupRemovedEvent *cev = static_cast<ICQ2000::GroupRemovedEvent*>(ev);
    if (&(cev->get_group()) == m_libicq2000_group)
      response(Gtk::RESPONSE_CANCEL);
  }
  else if (ev->getType() == ICQ2000::ContactListEvent::GroupChange)
  {
    ICQ2000::GroupChangeEvent *cev = static_cast<ICQ2000::GroupChangeEvent*>(ev);
    if (&(cev->get_group()) == m_libicq2000_group)
    {
      // update old name in dialog
      m_old_group_label.set_text( m_libicq2000_group->get_label());
    }
  }
}

