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

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>

#include "ickle.h"

#include <list>

using std::list;
using std::string;

// ------------------------------------------------------------------------------
//  RemoveGroupDialog
// ------------------------------------------------------------------------------

RemoveGroupDialog::RemoveGroupDialog(Gtk::Window& parent, ICQ2000::ContactTree::Group *gp)
  : Gtk::Dialog( _("Remove Group"), parent), m_libicq2000_group(gp),
    m_remove_all( _("Remove all contacts") ), m_move_all( _("Move all contacts to:") )
{
  set_position(Gtk::WIN_POS_CENTER);

  Gtk::Label *label;

  add_button(Gtk::Stock::REMOVE, Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  set_default_response(Gtk::RESPONSE_CANCEL);
  
  // libicq2000 callback
  icqclient.contactlist.connect( this, &RemoveGroupDialog::contactlist_cb );

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  label = manage( new Gtk::Label( _("What would you like to do with the contacts in that group?") ) );
  vbox->pack_start( *label );
  vbox->pack_start( m_remove_all );

  Gtk::RadioButton::Group group = m_remove_all.get_group();
  m_move_all.set_group( group );
  vbox->pack_start( m_move_all );

  // group list
  ICQ2000::ContactTree& ct = icqclient.getContactTree();
  ICQ2000::ContactTree::iterator curr = ct.begin();
  Gtk::ComboDropDown_Helpers::ComboDropDownList& cl = m_group_list.get_list()->children();
  while (curr != ct.end())
  {
    if ( &(*curr) != gp )
    {
      Gtk::ComboDropDownItem * item = Gtk::manage( new Gtk::ComboDropDownItem() );
      item->add_label( curr->get_label() );
      item->show_all();
      item->signal_select().connect( SigC::bind( SigC::slot( *this, &RemoveGroupDialog::selected_group_cb ), &(*curr) ) );
      cl.push_back( *item );
      m_group_list.set_item_string( *item, curr->get_label() );
      m_group_map[ curr->get_id() ] = item;
    }
    
    ++curr;
  }

  m_group_list.set_value_in_list();

  if (cl.empty())
  {
    m_remove_all.set_active(true);
    m_move_all.set_sensitive(false);
    m_group_list.set_sensitive(false);
  }

  Gtk::HBox *hbox = manage( new Gtk::HBox() );
  hbox->pack_start( m_group_list, Gtk::PACK_SHRINK );
  vbox->pack_start( *hbox );

  set_border_width(10);
  show_all();
}

void RemoveGroupDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    if (m_remove_all.get_active())
    {
      // remove contacts
      while (!m_libicq2000_group->empty())
	m_libicq2000_group->remove( (*m_libicq2000_group->begin())->getUIN() );
    }
    else
    {
      // move contacts
      while (!m_libicq2000_group->empty())
      {
	icqclient.getContactTree().relocate_contact( * m_libicq2000_group->begin(),
						     * m_libicq2000_group,
						     * m_selected_group );
      }
      
    }
    
    // remove the group
    ICQ2000::ContactTree& ct = icqclient.getContactTree();
    ct.remove_group( *m_libicq2000_group );
    
    // contactlist_cb will handle emitting destroy
  }
  else
  {
    delete this;
  }
}

void RemoveGroupDialog::contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  // could do add as well, if we're feeling really keen..

  if (ev->getType() == ICQ2000::ContactListEvent::GroupRemoved)
  {
    ICQ2000::GroupRemovedEvent *cev = static_cast<ICQ2000::GroupRemovedEvent*>(ev);
    if (&(cev->get_group()) == m_libicq2000_group)
    {
      // kill this dialog
      delete this;
    }
    else
    {
      // remove from list
      unsigned int id = cev->get_group().get_id();
      if ( m_group_map.count(id) )
      {
	m_group_list.get_list()->children().remove( * m_group_map[id] );
	m_group_map.erase(id);
      }

      if (m_group_list.get_list()->children().empty())
      {
	m_remove_all.set_active(true);
	m_move_all.set_sensitive(false);
	m_group_list.set_sensitive(false);
      }
    }
  }
  else if (ev->getType() == ICQ2000::ContactListEvent::GroupChange)
  {
    ICQ2000::GroupChangeEvent *cev = static_cast<ICQ2000::GroupChangeEvent*>(ev);
    if (&(cev->get_group()) != m_libicq2000_group)
    {
      unsigned int id = cev->get_group().get_id();
      if ( m_group_map.count(id) )
      {
	// update in list
	Gtk::ComboDropDownItem * item = m_group_map[id];
	item->remove();
	item->add_label( cev->get_group().get_label() );
	m_group_list.set_item_string( *item, cev->get_group().get_label() );
      }
    }
  }
}

void RemoveGroupDialog::selected_group_cb(ICQ2000::ContactTree::Group *gp)
{
  m_selected_group = gp;

  // select move to, since that's probably what they want
  m_move_all.set_active(true);
}
