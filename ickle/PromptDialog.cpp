/*
 * PromptDialog
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

#include "PromptDialog.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/main.h>
#include <gtkmm/buttonbox.h>

#include "ickle.h"

using std::string;

PromptDialog::PromptDialog(Gtk::Window& parent, Gtk::MessageType t, const Glib::ustring& msg, bool modal)
  : Gtk::MessageDialog(parent, msg, t,
		       t == Gtk::MESSAGE_QUESTION ? Gtk::BUTTONS_YES_NO : Gtk::BUTTONS_OK),
    m_type(t),
    m_modal(modal)
{
  set_modal(modal);
  set_position(Gtk::WIN_POS_CENTER);
  
  switch(t)
  {
  case Gtk::MESSAGE_INFO:
    set_title( _("ickle information") );
    break;
  case Gtk::MESSAGE_WARNING:
    set_title( _("ickle warning") );
    break;
  case Gtk::MESSAGE_QUESTION:
    set_title( _("ickle question") );
    break;
  case Gtk::MESSAGE_ERROR:
    set_title( _("ickle error") );
    break;
  }

  set_border_width(10);
  show_all();
}

void PromptDialog::on_response(int response_id)
{
  if (m_modal)
  {
    // pass to base on_response
    Gtk::MessageDialog::on_response(response_id);
  }
  else
  {
    // destroy me
    delete this;
  }
}
