/*
 * ResendDialog
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

#include "ResendDialog.h"

#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/main.h>
#include <gtk--/buttonbox.h>

#include "sstream_fix.h"
#include "main.h"
#include <libicq2000/Client.h>

using std::ostringstream;

using std::string;
using std::ostringstream;
using std::endl;

ResendDialog::ResendDialog(Gtk::Window *parent, ICQ2000::ICQMessageEvent *ev)
{
  set_position(GTK_WIN_POS_MOUSE);
  if (parent) set_transient_for (*parent);

  // take a copy of the event, for resending
  m_event = ev->copy();

  Gtk::HBox *hbox = get_action_area();
  hbox->set_border_width(0);
  Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  Gtk::Button *button;
  hbox->pack_start( *hbbox );

  set_title("Resend Message");

  button = manage( new Gtk::Button("Send as urgent") );
  button->clicked.connect( slot( this, &ResendDialog::resend_as_urgent_cb ) );
  hbbox->pack_start(*button, true, false, 0);

  button = manage( new Gtk::Button("Send to contact list") );
  button->clicked.connect( slot( this, &ResendDialog::resend_as_tocontactlist_cb ) );
  hbbox->pack_start(*button, true, false, 0);

  button = manage( new Gtk::Button("Cancel") );
  button->clicked.connect( destroy.slot() );
  hbbox->pack_start(*button, true, false, 0);

  ostringstream ostr;
  ostr << "Your message to " << ev->getContact()->getNameAlias()
       << " wasn't received as the user is "
       << (ev->getDeliveryFailureReason() == ICQ2000::MessageEvent::Failed_Occupied
	   ? "Occupied" : "in Do not Disturb")
       << "." << endl << endl
       << "Their away message is:" << endl
       << ev->getAwayMessage() << endl << endl
       << "You should resend the message as 'Urgent' or 'to Contact List'" << endl;
  
  Gtk::Label *label = manage( new Gtk::Label( ostr.str(), 0 ) );
  label->set_justify(GTK_JUSTIFY_FILL);
  label->set_line_wrap(true);
  Gtk::VBox *vbox = get_vbox();
  vbox->pack_start( *label, true, true );
  vbox->set_spacing(10);

  set_border_width(10);
  show_all();
}

ResendDialog::~ResendDialog()
{
  if (m_event != NULL) delete m_event;
}

void ResendDialog::resend_as_urgent_cb() {
  m_event->setUrgent(true);
  icqclient.SendEvent(m_event);
  m_event = NULL;
  destroy.emit();
}

void ResendDialog::resend_as_tocontactlist_cb() {
  m_event->setToContactList(true);
  icqclient.SendEvent(m_event);
  m_event = NULL;
  destroy.emit();
}
