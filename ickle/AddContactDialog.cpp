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

#include "AddContactDialog.h"

#include "main.h"
#include <libicq2000/Client.h>

#include <gtkmm/table.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>
#include <gtkmm/sizegroup.h>

#include "ickle.h"

AddContactDialog::AddContactDialog(Gtk::Window& parent)
  : Gtk::Dialog( _("Add Contact"), parent),
    m_icq_contact( _("An ICQ contact (has a uin)")),
    m_mobile_contact( _("A Mobile contact (cellular)")),
    m_uin_label( _("User Number (UIN)"), 0.0, 0.5),
    m_alert_check( _("Alert Contact"), 0),
    m_mode_frame( _("Type of contact") ),
    m_icq_frame( _("ICQ Contacts") ),
    m_mobile_frame( _("Mobile Contacts") ),
    m_group_frame( _("Group") ),
    m_alias_label( _("Alias"), 0.0, 0.5),
    m_mobileno_label( _("Mobile Number"), 0.0, 1.0),
    m_selected_group(NULL), m_ok_button( * add_button(Gtk::Stock::ADD, Gtk::RESPONSE_OK) )
{
  set_position(Gtk::WIN_POS_CENTER);

  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  set_default_response(Gtk::RESPONSE_OK);

  m_uin_entry.set_activates_default(true);
  m_alias_entry.set_activates_default(true);

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  Gtk::VBox *vbox2;
  Gtk::Table *table;

  Glib::RefPtr< Gtk::SizeGroup > size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);

  icqclient.contactlist.connect( this, &AddContactDialog::contactlist_cb );

  // -- mode selection frame

  vbox2 = manage( new Gtk::VBox() );
  m_icq_contact.signal_clicked().connect( SigC::slot( *this, &AddContactDialog::update_stuff ) );
  m_mobile_contact.signal_clicked().connect( SigC::slot( *this, &AddContactDialog::update_stuff ) );

  Gtk::RadioButton::Group group = m_icq_contact.get_group();
  m_mobile_contact.set_group( group );
  m_icq_contact.set_active(true);
  vbox2->set_border_width(5);
  vbox2->pack_start( m_icq_contact );
  vbox2->pack_start( m_mobile_contact );

  m_mode_frame.set_border_width(0);
  m_mode_frame.add( *vbox2 );
  vbox->pack_start( m_mode_frame );

  // -- icq frame
  table = manage( new Gtk::Table( 2, 2, false ) );
  
  table->attach( m_uin_label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5 );
  m_uin_entry.signal_changed().connect( SigC::slot( *this, &AddContactDialog::uin_changed_cb ) );
  m_uin_entry.set_max_length(12);
  m_uin_entry.set_width_chars(12);
  size_group->add_widget( m_uin_entry );

  table->set_border_width(5);
  table->attach( m_uin_entry, 1, 2, 0, 1, Gtk::AttachOptions(0), Gtk::FILL | Gtk::EXPAND );
  table->attach( m_alert_check, 0, 2, 1, 2, Gtk::FILL, Gtk::FILL | Gtk::EXPAND );
  m_icq_frame.set_border_width(0);
  m_icq_frame.add( *table );
  vbox->pack_start( m_icq_frame );

  // -- mobile frame
  table = manage( new Gtk::Table( 2, 2, false ) );

  table->attach( m_alias_label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5);

  size_group->add_widget( m_alias_entry );
  table->attach( m_alias_entry, 1, 2, 0, 1, Gtk::AttachOptions(0), Gtk::FILL | Gtk::EXPAND);

  table->attach( m_mobileno_label, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 5);

  m_mobileno_entry.signal_changed().connect( SigC::slot( *this, &AddContactDialog::mobileno_changed_cb ) );
  size_group->add_widget( m_mobileno_entry );
  table->attach( m_mobileno_entry, 1, 2, 1, 2, Gtk::AttachOptions(0), Gtk::FILL | Gtk::EXPAND);

  m_mobile_frame.set_border_width(0);
  m_mobile_frame.add( *table );
  table->set_border_width(5);
  vbox->pack_start( m_mobile_frame );

  // -- group frame
  table = manage( new Gtk::Table( 2, 1, false ) );
  
  // group list
  ICQ2000::ContactTree& ct = icqclient.getContactTree();

  if ( ! ct.empty() )
  {
    ICQ2000::ContactTree::iterator curr = ct.begin();
    Gtk::ComboDropDown_Helpers::ComboDropDownList& cl = m_group_list.get_list()->children();
    while (curr != ct.end())
    {
      Gtk::ComboDropDownItem * item = Gtk::manage( new Gtk::ComboDropDownItem() );
      item->add_label( curr->get_label() );
      item->show_all();
      item->signal_select().connect( SigC::bind( SigC::slot( *this, &AddContactDialog::selected_group_cb ), &(*curr) ) );
      cl.push_back( *item );
      m_group_list.set_item_string( *item, curr->get_label() );
      m_group_map[ curr->get_id() ] = item;

      ++curr;
    }
  }
  else
  {
    std::list< std::string > strlist;
    strlist.push_back( _("New") );
    m_selected_group = NULL;
    m_group_list.set_popdown_strings( strlist );
  }

  m_group_list.set_value_in_list();

  table->attach( * manage( new Gtk::Label( _("Add to group"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );

  size_group->add_widget( m_group_list );
  table->attach( m_group_list, 1, 2, 0, 1, Gtk::AttachOptions(0), Gtk::FILL | Gtk::EXPAND );

  table->set_border_width(5);
  m_group_frame.set_border_width(0);
  m_group_frame.add( *table );
  vbox->pack_start( m_group_frame );

  set_border_width(10);
  show_all();
}

void AddContactDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    if (m_icq_contact.get_active())
    {
      unsigned int uin = ICQ2000::Contact::StringtoUIN(m_uin_entry.get_text());

      if (uin == 0)
	return;

      ICQ2000::ContactRef c = icqclient.getContact(uin);
      if (c.get() == NULL)
      {
	ICQ2000::ContactRef nc(new ICQ2000::Contact(uin));
	
	ICQ2000::ContactTree::Group * gp = ( m_selected_group != NULL ? m_selected_group : create_new_group() );
	gp->add(nc);

	if (m_alert_check.get_active())
	{
	  ICQ2000::UserAddEvent *ev = new ICQ2000::UserAddEvent(nc);
	  icqclient.SendEvent(ev);
	}

	// fetch user info
	icqclient.fetchDetailContactInfo(nc);
      }
    }
    else
    {
      ICQ2000::ContactRef nc(new ICQ2000::Contact(m_alias_entry.get_text()));
      nc->setMobileNo(m_mobileno_entry.get_text());
      
      ICQ2000::ContactTree::Group * gp = ( m_selected_group != NULL ? m_selected_group : create_new_group() );
      gp->add(nc);
    }
  }

  delete this;
}

ICQ2000::ContactTree::Group * AddContactDialog::create_new_group()
{
  ICQ2000::ContactTree::Group * gp = NULL;
  ICQ2000::ContactTree& ct = icqclient.getContactTree();
  ICQ2000::ContactTree::iterator curr = ct.begin();

  while (curr != ct.end())
  {
    if ((*curr).get_label() == _("New") )
    {
      gp = &(*curr);
      break;
    }

    ++curr;
  }

  if (gp == NULL)
    gp = &(ct.add_group( _("New") ));

  return gp;
}

void AddContactDialog::update_stuff()
{
  if (m_icq_contact.get_active())
  {
    // set uin stuff sensitive
    m_uin_label.set_sensitive(true);
    m_uin_entry.set_sensitive(true);
    m_alert_check.set_sensitive(true);

    // set mobile stuff insensitive
    m_alias_label.set_sensitive(false);
    m_alias_entry.set_sensitive(false);
    m_mobileno_label.set_sensitive(false);
    m_mobileno_entry.set_sensitive(false);

    unsigned int uin = ICQ2000::Contact::StringtoUIN(m_uin_entry.get_text());
    m_ok_button.set_sensitive( uin != 0 );
  }
  else
  {
    // set mobile stuff sensitive
    m_alias_label.set_sensitive(true);
    m_alias_entry.set_sensitive(true);
    m_mobileno_label.set_sensitive(true);
    m_mobileno_entry.set_sensitive(true);

    // set uin stuff insensitive
    m_uin_label.set_sensitive(false);
    m_uin_entry.set_sensitive(false);
    m_alert_check.set_sensitive(false);

    m_ok_button.set_sensitive( ! m_mobileno_entry.get_text().empty() );
  }
}

void AddContactDialog::uin_changed_cb()
{
  if (!m_icq_contact.get_active())
    return;
  
  unsigned int uin = ICQ2000::Contact::StringtoUIN(m_uin_entry.get_text());
  m_ok_button.set_sensitive( uin != 0 );
}

void AddContactDialog::mobileno_changed_cb()
{
  if (m_icq_contact.get_active())
    return;
  
  m_ok_button.set_sensitive( ! m_mobileno_entry.get_text().empty() );
}

void AddContactDialog::contactlist_cb(ICQ2000::ContactListEvent * ev)
{
  if (ev->getType() == ICQ2000::ContactListEvent::GroupRemoved)
  {
    ICQ2000::GroupRemovedEvent *cev = static_cast<ICQ2000::GroupRemovedEvent*>(ev);

    // remove from list
    unsigned int id = cev->get_group().get_id();
    if ( m_group_map.count(id) )
    {
      m_group_list.get_list()->children().remove( * m_group_map[id] );
      m_group_map.erase(id);
    }

    if (m_group_list.get_list()->children().empty())
    {
      /* if they empty the list, instead add the fictional "New" group */
      std::list< std::string > strlist;
      strlist.push_back( _("New") );
      m_selected_group = NULL;
      m_group_list.set_popdown_strings( strlist );
    }
    
  }
}

void AddContactDialog::selected_group_cb(ICQ2000::ContactTree::Group * gp)
{
  m_selected_group = gp;
}
