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

#include "RemoveGroupDialog.h"

#include "main.h"
#include <libicq2000/Client.h>

#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/buttonbox.h>

#include <list>

using std::list;
using std::string;

// ------------------------------------------------------------------------------
//  GroupItem
// ------------------------------------------------------------------------------

GroupItem::GroupItem(const ICQ2000::ContactTree::Group *gp, gfloat x, gfloat y)
  : Gtk::ListItem( gp->get_label(), x, y ), m_libicq2000_group(gp)
{ }

const ICQ2000::ContactTree::Group * GroupItem::get_group()
{
  return m_libicq2000_group;
}

// ------------------------------------------------------------------------------
//  RemoveGroupDialog
// ------------------------------------------------------------------------------

RemoveGroupDialog::RemoveGroupDialog(Gtk::Window *parent, ICQ2000::ContactTree::Group *gp)
  : Gtk::Dialog(), m_libicq2000_group(gp),
    m_ok("Remove"), m_cancel("Cancel"),
    m_remove_all("Remove all contacts"), m_move_all("Move all contacts to:")
{
  Gtk::Label *label;

  set_title("Remove Group");
  set_transient_for(*parent);

  m_ok.clicked.connect(slot(this,&RemoveGroupDialog::ok_cb));
  m_cancel.clicked.connect( destroy.slot() );

  // libicq2000 callback
  icqclient.contactlist.connect( slot( this, &RemoveGroupDialog::contactlist_cb ) );

  Gtk::HBox *hbox = get_action_area();
  hbox->set_border_width(0);
  Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  hbbox->pack_start(m_ok, true, true, 0);
  hbbox->pack_start(m_cancel, true, true, 0);
  hbox->pack_start( *hbbox );

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  label = manage( new Gtk::Label("What would you like to do with the contacts in that group?") );
  vbox->pack_start( *label );
  vbox->pack_start( m_remove_all );
  m_move_all.set_group( m_remove_all.group() );
  vbox->pack_start( m_move_all );

  // group list
  ICQ2000::ContactTree& ct = icqclient.getContactTree();
  ICQ2000::ContactTree::const_iterator curr = ct.begin();
  Gtk::List *item_list = m_group_list.get_list();
  while (curr != ct.end()) {
    if ( &(*curr) != gp ) {
      GroupItem *item = manage( new GroupItem( &(*curr) ) );
      item->show();
      item_list->add( *item );
    }
    ++curr;
  }
  m_group_list.set_value_in_list(true, false);

  hbox = manage( new Gtk::HBox() );
  hbox->pack_end(m_group_list, false);
  vbox->pack_start( *hbox );

  set_border_width(10);
  show_all();
}

void RemoveGroupDialog::ok_cb()
{
  if (m_remove_all.get_active()) {
    // remove contacts
    while (!m_libicq2000_group->empty())
      m_libicq2000_group->remove( (*m_libicq2000_group->begin())->getUIN() );
  } else {
    // move contacts
  }
  
  // remove the group
  ICQ2000::ContactTree& ct = icqclient.getContactTree();
  ct.remove_group( *m_libicq2000_group );

  // contactlist_cb will handle emitting destroy
}

void RemoveGroupDialog::contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  // could do add as well, if we're feeling really keen..

  if (ev->getType() == ICQ2000::ContactListEvent::GroupRemoved) {
    ICQ2000::GroupRemovedEvent *cev = static_cast<ICQ2000::GroupRemovedEvent*>(ev);
    if (&(cev->get_group()) == m_libicq2000_group) {
      // kill this dialog
      destroy.emit();
    } else {
      // remove from list
      Gtk::List_Helpers::ItemList& il = m_group_list.get_list()->items();
      Gtk::List_Helpers::ItemList::iterator curr = il.begin();
      while (curr != il.end()) {
	Gtk::ListItem *li = *curr;
	GroupItem *gli;
	if ((gli = dynamic_cast<GroupItem*>(li)) != NULL) {
	  if (gli->get_group() == &(cev->get_group())) {
	    // remove this one
	    il.erase(curr);
	    break;
	  }
	}
	++curr;
      }
    }
  } else if (ev->getType() == ICQ2000::ContactListEvent::GroupChange) {
    ICQ2000::GroupChangeEvent *cev = static_cast<ICQ2000::GroupChangeEvent*>(ev);
    if (&(cev->get_group()) != m_libicq2000_group) {
      // update in list
    }
  }
}
