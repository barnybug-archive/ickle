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

#include <gtk--/buttonbox.h>
#include <gtk--/label.h>

#include "sstream_fix.h"

using std::ostringstream;

RemoveContactDialog::RemoveContactDialog(Gtk::Window * parent, const ICQ2000::ContactRef& c)
  : Gtk::Dialog(),
    m_ok("OK"), m_cancel("Cancel"),
    m_contact(c)
{
  Gtk::Label *label;

  set_title("Remove Contact");
  set_transient_for (*parent);

  m_ok.clicked.connect(slot(this,&RemoveContactDialog::ok_cb));
  m_cancel.clicked.connect( destroy.slot() );

  // libicq2000 callbacks
  icqclient.contactlist.connect( slot( this, &RemoveContactDialog::contactlist_cb ) );

  Gtk::HBox *hbox = get_action_area();
  hbox->set_border_width(0);
  Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  hbbox->pack_start(m_ok, true, true, 0);
  hbbox->pack_start(m_cancel, true, true, 0);
  hbox->pack_start( *hbbox );

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
  if (ev->getType() == ICQ2000::ContactListEvent::UserRemoved) {
    ICQ2000::UserRemovedEvent *cev = static_cast<ICQ2000::UserRemovedEvent*>(ev);
    ICQ2000::ContactRef c = cev->getContact();
    if (m_contact->getUIN() == c->getUIN()) destroy.emit();
  }
}

void RemoveContactDialog::ok_cb() {
  ICQ2000::ContactTree& ct = icqclient.getContactTree();
  ct.remove( m_contact->getUIN() );
  destroy.emit();
}
