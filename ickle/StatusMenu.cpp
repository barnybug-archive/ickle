/* $Id: StatusMenu.cpp,v 1.1 2002-01-16 12:58:40 barnabygray Exp $
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

#include "StatusMenu.h"

#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/pixmap.h>
#include <gtk--/checkmenuitem.h>

#include "Icons.h"

using namespace ICQ2000;

StatusMenuItem::StatusMenuItem()
{
}

StatusMenuItem::StatusMenuItem(Status st, bool inv)
{
  add_status(st,inv);
};

void StatusMenuItem::add_status(Status st, bool inv)
{
  Gtk::ImageLoader *p = g_icons.IconForStatus(st, inv);

  Gtk::Pixmap* pmap = manage( new Gtk::Pixmap( p->pix(), p->bit() ) );
  Gtk::Label* label = manage( new Gtk::Label( Status_text[st] , 1.0) );
  Gtk::HBox* box = manage( new Gtk::HBox(false,5) );

  box->pack_start(*label);
  box->pack_end(*pmap,false);
  box->show_all();

  add(*box);
}

InvisibleStatusMenuItem::InvisibleStatusMenuItem()
{
  Gtk::ImageLoader *p = g_icons.IconForStatus(STATUS_ONLINE, true);

  Gtk::Pixmap* pmap = manage( new Gtk::Pixmap( p->pix(), p->bit() ) );
  Gtk::Label* label = manage( new Gtk::Label( "Invisible" , 1.0) );
  Gtk::HBox* box = manage( new Gtk::HBox(false,5) );

  box->pack_start(*label);
  box->pack_end(*pmap,false);
  box->show_all();

  add(*box);
}

StatusMenu::StatusMenu()
  : m_current_status(STATUS_OFFLINE), m_current_invisible(false)
{
  g_icons.icons_changed.connect( slot( this, &StatusMenu::icons_changed_cb ) );
  icons_changed_cb();
  set_submenu(m_menu);
}

void StatusMenu::icons_changed_cb()
{
  using namespace Gtk::Menu_Helpers;
  
  // menu header item
  remove();
  add_status(m_current_status, m_current_invisible);

  // menu list
  MenuList& sl = m_menu.items();
  sl.clear();
  sl.push_back(* menu_status_widget( STATUS_ONLINE ) );
  sl.push_back(* menu_status_widget( STATUS_AWAY ) );
  sl.push_back(* menu_status_widget( STATUS_NA ) );
  sl.push_back(* menu_status_widget( STATUS_DND ) );
  sl.push_back(* menu_status_widget( STATUS_OCCUPIED ) );
  sl.push_back(* menu_status_widget( STATUS_FREEFORCHAT ) );
  sl.push_back(* menu_status_widget( STATUS_OFFLINE ) );
  sl.push_back( SeparatorElem() );
  sl.push_back(* menu_status_inv_widget() );
  m_menu.show_all();

}

Gtk::MenuItem* StatusMenu::menu_status_widget( Status s ) {
  StatusMenuItem *mi = manage( new StatusMenuItem( s, false ) );
  mi->activate.connect( bind(slot(this,&StatusMenu::menu_activate_cb),s) );
  return mi;
}
  
Gtk::MenuItem* StatusMenu::menu_status_inv_widget() {
  InvisibleStatusMenuItem *mi = manage( new InvisibleStatusMenuItem() );
  mi->toggled.connect( bind(slot(this,&StatusMenu::inv_toggled_cb), mi) );
  return mi;
}

void StatusMenu::inv_toggled_cb(InvisibleStatusMenuItem *mi) 
{
  status_changed_invisible.emit(mi->is_active());
}

void StatusMenu::menu_activate_cb(Status st) 
{
  status_changed_status.emit(st);
}

void StatusMenu::status_changed_cb(Status st, bool inv)
{
  remove();
  add_status(st, inv);
  m_current_status = st;
  m_current_invisible = inv;
  
}
