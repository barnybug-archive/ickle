/*
 * SetAutoResponseDialog: Modeless dialog for changing auto response
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

#ifndef SETAWAYMSGDIALOG_H
#define SETAWAYMSGDIALOG_H

#include <gtkmm/main.h>
#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/optionmenu.h>

#include <string>

#include <sigc++/signal.h>

class SetAutoResponseDialog : public Gtk::Dialog
{
 private:
  Gtk::OptionMenu m_autoresponse_option;
  Gtk::TextView m_msg_textview;
  Gtk::ScrolledWindow m_msg_scr_win;
  Gtk::Tooltips m_tooltip;
  Gtk::Button * m_ok;

  unsigned int m_timeout;
  SigC::Connection timeout_connection;

  void build_optionmenu();

 public:
  SetAutoResponseDialog(Gtk::Window& parent, const std::string& prev_msg, bool timeout);

  bool auto_timeout();
  void cancel_timeout();

  virtual void on_response(int response_id);
  virtual bool on_key_press_event(GdkEventKey* ev);
  virtual bool on_button_press_event(GdkEventButton* ev);

  void activate_menu_item_cb(int);
  void edit_messages_cb();
  
  bool option_button_pressed(GdkEventButton *);

  SigC::Signal1<void, const std::string&> save_new_msg;
  SigC::Signal0<void> settings_dialog;
};

#endif
