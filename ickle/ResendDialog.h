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

#ifndef RESENDDIALOG_H
#define RESENDDIALOG_H

#include <gtk--/dialog.h>

#include <string>

#include <libicq2000/events.h>

class ResendDialog : public Gtk::Dialog {
 private:
  ICQ2000::ICQMessageEvent *m_event;

 protected:
  void resend_as_urgent_cb();
  void resend_as_tocontactlist_cb();
  void cancel_cb();

 public:
  ResendDialog(Gtk::Window *parent, ICQ2000::ICQMessageEvent *ev);
  ~ResendDialog();
};

#endif
