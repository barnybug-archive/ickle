/*
 * SendAuthReqDialog
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

#ifndef SENDAUTHREQDIALOG_H
#define SENDAUTHREQDIALOG_H

#include <gtk--/dialog.h>
#include <gtk--/button.h>
#include <gtk--/text.h>

#include <libicq2000/Contact.h>

class SendAuthReqDialog : public Gtk::Dialog {
 private:
  ICQ2000::ContactRef m_contact;
  Gtk::Button m_ok, m_cancel;
  Gtk::Text m_text;

 public:
  SendAuthReqDialog(Gtk::Window * parent, const ICQ2000::ContactRef& contact);

  void ok_cb();
};

#endif
