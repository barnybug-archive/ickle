/* $Id: StatusMenu.h,v 1.3 2002-06-25 18:08:35 barnabygray Exp $
 *
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

#ifndef STATUSMENU_H
#define STATUSMENU_H

#include <gtk--/menu.h>
#include <gtk--/menuitem.h>
#include <gtk--/checkmenuitem.h>
#include <sigc++/signal_system.h>

#include <libicq2000/constants.h>

class StatusMenuItem : public Gtk::MenuItem
{
 public:
  StatusMenuItem();
  StatusMenuItem(ICQ2000::Status st, bool inv);
  StatusMenuItem(const std::string& label, ICQ2000::Status st, bool inv);

  void add_status(const std::string& label, ICQ2000::Status st, bool inv);
  void add_status(ICQ2000::Status st, bool inv);
};

class InvisibleStatusMenuItem : public Gtk::CheckMenuItem
{
 public:
  InvisibleStatusMenuItem();
};

class StatusMenu : public StatusMenuItem {
 private:
  ICQ2000::Status m_current_status;
  bool m_current_invisible;

  Gtk::Menu m_menu;

 protected:
  Gtk::MenuItem* menu_status_widget(ICQ2000::Status st, bool set_inv);
  Gtk::MenuItem* menu_status_nice_inv_widget();
  Gtk::MenuItem* menu_status_inv_widget();

  void inv_toggled_cb(InvisibleStatusMenuItem *mi);
  void menu_activate_cb(ICQ2000::Status st);
  void menu_activate_inv_cb(ICQ2000::Status st, bool inv);

  void build_list();

 public:
  StatusMenu();
  
  void icons_changed_cb();
  void settings_changed_cb(const std::string& s);
  void status_changed_cb(ICQ2000::Status st, bool inv);
  void connecting();
  void set_status(ICQ2000::Status st, bool inv);

  SigC::Signal2<void,ICQ2000::Status,bool> status_changed_status_inv;
  SigC::Signal1<void,ICQ2000::Status> status_changed_status;
  SigC::Signal1<void,bool> status_changed_invisible;
};

#endif
