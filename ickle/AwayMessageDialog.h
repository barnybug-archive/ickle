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

#ifndef AWAYMESSAGEDIALOG_H
#define AWAYMESSAGEDIALOG_H

#include <gtk--/window.h>
#include <gtk--/box.h>

#include <sigc++/signal_system.h>

#include "Contact.h"
#include "events.h"

using namespace ICQ2000;

class AwayMessageDialog : public Gtk::Window {
 private:
  Gtk::VBox m_awayvbox;

 public:
  AwayMessageDialog(Contact *c);

  void away_message_cb(AwayMsgEvent *ev);

};

#endif
