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

#include <gtk--/box.h>
#include <gtk--/table.h>
#include <gtk--/label.h>
#include <gtk--/buttonbox.h>

RenameGroupDialog::RenameGroupDialog(Gtk::Window *parent, ICQ2000::ContactTree::Group *gp)
  : Gtk::Dialog(), m_libicq2000_group( gp ),
    m_ok("Rename"), m_cancel("Cancel")
{
  Gtk::Label *label;

  set_title("Rename Group");
  set_transient_for(*parent);

  m_ok.clicked.connect(slot(this,&RenameGroupDialog::ok_cb));
  m_cancel.clicked.connect( destroy.slot() );

  // libicq2000 callbacks
  icqclient.contactlist.connect( slot( this, &RenameGroupDialog::contactlist_cb ) );

  Gtk::HBox *hbox = get_action_area();
  hbox->set_border_width(0);
  Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  hbbox->pack_start(m_ok, true, true, 0);
  hbbox->pack_start(m_cancel, true, true, 0);
  hbox->pack_start( *hbbox );

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  Gtk::Table *table = manage( new Gtk::Table(2, 2) );

  label = manage( new Gtk::Label("Old group label") );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );
  m_old_group_label.set_text( gp->get_label() );
  m_old_group_label.set_editable(false);
  table->attach( m_old_group_label, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );

  label = manage( new Gtk::Label("New group label") );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );
  table->attach( m_group_label, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );

  vbox->pack_start( *table );

  set_border_width(10);
  show_all();
  m_group_label.grab_focus();
}

void RenameGroupDialog::ok_cb()
{
  m_libicq2000_group->set_label( m_group_label.get_text() );
  destroy.emit();
}

void RenameGroupDialog::contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  if (ev->getType() == ICQ2000::ContactListEvent::GroupRemoved) {
    ICQ2000::GroupRemovedEvent *cev = static_cast<ICQ2000::GroupRemovedEvent*>(ev);
    if (&(cev->get_group()) == m_libicq2000_group) destroy.emit();
  } else if (ev->getType() == ICQ2000::ContactListEvent::GroupChange) {
    ICQ2000::GroupChangeEvent *cev = static_cast<ICQ2000::GroupChangeEvent*>(ev);
    if (&(cev->get_group()) == m_libicq2000_group) {
      // update old name in dialog
      m_old_group_label.set_text( m_libicq2000_group->get_label());
    }
  }
}

