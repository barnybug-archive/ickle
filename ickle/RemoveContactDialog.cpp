/*
 * RemoveContactDialog
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

#include "RemoveContactDialog.h"

#include "main.h"
#include <libicq2000/Client.h>

#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>
#include <gtkmm/stock.h>

#include "sstream_fix.h"

using std::ostringstream;

RemoveContactDialog::RemoveContactDialog(Gtk::Window& parent, const ICQ2000::ContactRef& c)
  : Gtk::Dialog("Remove Contact", parent),
    m_contact(c)
{
  set_position(Gtk::WIN_POS_CENTER);

  Gtk::Label *label;

  add_button(Gtk::Stock::REMOVE, Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  // libicq2000 callbacks
  icqclient.contactlist.connect( this, &RemoveContactDialog::contactlist_cb );

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);

  ostringstream ostr;
  ostr << "Are you sure you want to remove " << c->getAlias();
  if (c->isICQContact()) ostr << " (" << c->getUIN() << ")";
  ostr << "?";
  label = manage( new Gtk::Label( ostr.str() ) );
  vbox->pack_start( *label );

  set_border_width(10);
  show_all();
}

void RemoveContactDialog::contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  // pick up remove from elsewhere, and kill the dialog
  if (ev->getType() == ICQ2000::ContactListEvent::UserRemoved)
  {
    ICQ2000::UserRemovedEvent *cev = static_cast<ICQ2000::UserRemovedEvent*>(ev);
    ICQ2000::ContactRef c = cev->getContact();
    if (m_contact->getUIN() == c->getUIN())
      delete this;
  }
}

void RemoveContactDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    ICQ2000::ContactTree& ct = icqclient.getContactTree();
    ct.remove( m_contact->getUIN() );
    //  destroy - not necessary, will be caught from signals
  }
  else
  {
    delete this;
  }
    
}
