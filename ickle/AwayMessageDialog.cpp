/*
 * AwayMessageDialog
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

#include "AwayMessageDialog.h"

#include <gtk--/box.h>
#include <gtk--/scrolledwindow.h>

AwayMessageDialog::AwayMessageDialog() {
  Gtk::VBox *box = manage( new Gtk::VBox() );

  Gtk::ScrolledWindow *scrolled_window = manage(new Gtk::ScrolledWindow());
  scrolled_window->set_usize(250, 150);
  scrolled_window->set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  scrolled_window->add_with_viewport(m_awayvbox);

  box->pack_end(*scrolled_window);
}

void AwayMessageDialog::away_message_cb(AwayMsgEvent *ev) {
  
}

