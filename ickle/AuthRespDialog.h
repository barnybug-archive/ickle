/*
 * AuthRespDialog
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

#ifndef AUTHRESPDIALOG_H
#define AUTHRESPDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/textview.h>
#include <gtkmm/label.h>

#include <libicq2000/Contact.h>

#include <MessageEvent.h>

class AuthRespDialog : public Gtk::Dialog
{
 private:
  ICQ2000::ContactRef m_contact;
  Gtk::Button m_ok, m_cancel;
  Gtk::RadioButton m_grant, m_refuse;
  Gtk::Label m_label;
  Gtk::TextView m_textview;

  void grant_clicked_cb();
  void refuse_clicked_cb();

 protected:
  virtual void on_response(int response_id);

 public:
  AuthRespDialog(Gtk::Window& parent, const ICQ2000::ContactRef& contact, AuthReqICQMessageEvent *ev);
};

#endif
