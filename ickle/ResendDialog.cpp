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

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/main.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>

#include "main.h"
#include <libicq2000/Client.h>

#include "ickle.h"
#include "ucompose.h"

using std::string;

enum
{
  Response_Resend_Urgent        = 1,
  Response_Resend_ToContactList = 2
};

ResendDialog::ResendDialog(Gtk::Window& parent, ICQ2000::ICQMessageEvent *ev)
  : Gtk::Dialog( _("Resend message"), parent)
{
  set_position(Gtk::WIN_POS_MOUSE);

  // take a copy of the event, for resending
  m_event = ev->copy();

  add_button( _("Send as urgent"), Response_Resend_Urgent);
  add_button( _("Send to contact list"), Response_Resend_ToContactList);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  Gtk::Label *label;
  label = manage( new Gtk::Label( String::ucompose( _("Your message to %1 wasn't received as the user is %2.\n\n"
						      "Their away message is:\n"
						      "%2\n\n"
						      "You should resend the message as 'Urgent' or 'to Contact List'\n"),
						    Glib::ustring(ev->getContact()->getNameAlias()),
						    Glib::ustring((ev->getDeliveryFailureReason() == ICQ2000::MessageEvent::Failed_Occupied
						     ? _("Occupied") : _("in Do not Disturb"))),
						    Glib::ustring(ev->getAwayMessage()) ),
				  0.0, 0.5 ) );
  label->set_justify(Gtk::JUSTIFY_FILL);
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

void ResendDialog::on_response(int response_id)
{
  if (response_id == Response_Resend_Urgent)
  {
    m_event->setUrgent(true);
    icqclient.SendEvent(m_event);
    m_event = NULL;
  }
  else if (response_id == Response_Resend_ToContactList)
  {
    m_event->setToContactList(true);
    icqclient.SendEvent(m_event);
    m_event = NULL;
  }

  delete this;
}
