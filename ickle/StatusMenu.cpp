/* $Id: StatusMenu.cpp,v 1.7 2003-01-02 16:40:01 barnabygray Exp $
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

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/checkmenuitem.h>

#include "Icons.h"
#include "Settings.h"
#include "main.h"

using namespace ICQ2000;

using std::string;

StatusMenuItem::StatusMenuItem()
{
}

StatusMenuItem::StatusMenuItem(Status st, bool inv)
{
  add_status(st,inv);
};

StatusMenuItem::StatusMenuItem(const string& label, Status st, bool inv)
{
  add_status(label,st,inv);
}

void StatusMenuItem::add_status(Status st, bool inv)
{
  if (g_settings.getValueBool("status_classic_invisibility") &&
      inv && st != STATUS_OFFLINE) {
    add_status("Invisible", st, inv);
  } else {
    add_status(Status_text[st], st, inv);
  }
}

void StatusMenuItem::add_status(const string& lbl, Status st, bool inv)
{
  Glib::RefPtr<Gdk::Pixbuf> p = g_icons.get_icon_for_status(st, inv);

  Gtk::Image* img = manage( new Gtk::Image( p ) );
  Gtk::Label* label = manage( new Gtk::Label( lbl, 1.0 ) );
  Gtk::HBox* box = manage( new Gtk::HBox( false, 5 ) );

  box->pack_start(*label, Gtk::PACK_EXPAND_WIDGET);
  label->set_alignment( 1.0, 0.5 );
  box->pack_start(*img,   Gtk::PACK_SHRINK);

  box->show_all();

  set_right_justified(true);
  add(*box);
}

InvisibleStatusMenuItem::InvisibleStatusMenuItem()
{
  Glib::RefPtr<Gdk::Pixbuf> p = g_icons.get_icon_for_status(STATUS_ONLINE, true);

  Gtk::Image* img = manage( new Gtk::Image( p ) );
  Gtk::Label* label = manage( new Gtk::Label( "Invisible", 1.0) );
  Gtk::HBox* box = manage( new Gtk::HBox( false, 5 ) );

  box->pack_start(*label, Gtk::PACK_EXPAND_WIDGET);
  label->set_alignment( 1.0, 0.5 );
  box->pack_start(*img,   Gtk::PACK_SHRINK);

  box->show_all();

  set_right_justified(true);
  add(*box);
}

StatusMenu::StatusMenu()
  : m_current_status(STATUS_OFFLINE), m_current_invisible(false)
{
  g_icons.icons_changed.connect( SigC::slot( *this, &StatusMenu::icons_changed_cb ) );
  g_settings.settings_changed.connect( SigC::slot( *this, &StatusMenu::settings_changed_cb ) );
  build_list();
  set_submenu(m_menu);
}

void StatusMenu::icons_changed_cb()
{
  // TODO build_list();
}

void StatusMenu::settings_changed_cb(const string& s)
{
  //  if (s == "status_classic_invisibility")
    // TODO build_list();
}

void StatusMenu::build_list()
{
  using namespace Gtk::Menu_Helpers;
  
  // menu header item
  remove();
  add_status(m_current_status, m_current_invisible);

  bool cl_inv = g_settings.getValueBool("status_classic_invisibility");

  // menu list
  m_menu.items().clear();
  m_menu.append( * menu_status_widget( STATUS_ONLINE, cl_inv ) );
  m_menu.append( * menu_status_widget( STATUS_AWAY, cl_inv ) );
  m_menu.append( * menu_status_widget( STATUS_NA, cl_inv ) );
  m_menu.append( * menu_status_widget( STATUS_DND, cl_inv ) );
  m_menu.append( * menu_status_widget( STATUS_OCCUPIED, cl_inv ) );
  m_menu.append( * menu_status_widget( STATUS_FREEFORCHAT, cl_inv ) );
  if (cl_inv) {
    m_menu.append( * menu_status_nice_inv_widget() );
    m_menu.append( * menu_status_widget( STATUS_OFFLINE, cl_inv ) );
  } else {
    m_menu.append( * menu_status_widget( STATUS_OFFLINE, cl_inv ) );
    m_menu.items().push_back( SeparatorElem() );
    m_menu.append( * menu_status_inv_widget() );
  }

  m_menu.show_all();
}

Gtk::MenuItem* StatusMenu::menu_status_widget( Status s, bool set_inv )
{
  StatusMenuItem *mi = manage( new StatusMenuItem( s, false ) );
  if (set_inv)
    mi->signal_activate().connect( SigC::bind(SigC::bind(SigC::slot(*this,&StatusMenu::menu_activate_inv_cb), false), s) );
  else
    mi->signal_activate().connect( SigC::bind(SigC::slot(*this,&StatusMenu::menu_activate_cb),s) );

  return mi;
}
  
Gtk::MenuItem* StatusMenu::menu_status_nice_inv_widget()
{
  StatusMenuItem *mi = manage( new StatusMenuItem( "Invisible", STATUS_ONLINE, true ) );
  mi->signal_activate().connect( SigC::bind(SigC::bind(SigC::slot(*this,&StatusMenu::menu_activate_inv_cb), true), STATUS_ONLINE) );
  return mi;
}
  
Gtk::MenuItem* StatusMenu::menu_status_inv_widget()
{
  InvisibleStatusMenuItem *mi = manage( new InvisibleStatusMenuItem() );
  mi->signal_toggled().connect( SigC::bind(SigC::slot(*this,&StatusMenu::inv_toggled_cb), mi) );
  return mi;
}

void StatusMenu::inv_toggled_cb(InvisibleStatusMenuItem *mi) 
{
  status_changed_invisible.emit(mi->get_active());
}

void StatusMenu::menu_activate_cb(Status st) 
{
  status_changed_status.emit(st);
}

void StatusMenu::menu_activate_inv_cb(Status st, bool inv) 
{
  status_changed_status_inv.emit(st, inv);
}

void StatusMenu::status_changed_cb(Status st, bool inv)
{
  set_status(st,inv);
}

void StatusMenu::set_status(Status st, bool inv)
{
  remove();
  add_status(st, inv);
  m_current_status = st;
  m_current_invisible = inv;
}

void StatusMenu::connecting()
{
  remove();

  Gtk::Label* label = manage( new Gtk::Label("Connecting...") );
  Gtk::HBox* box = manage( new Gtk::HBox(false, 5) );

  box->pack_start(*label);
  box->show_all();

  add(*box);
}
