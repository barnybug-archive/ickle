/*
 * SetAutoResponseDialog
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

#include "SetAutoResponseDialog.h"

#include <gtkmm/table.h>
#include <gtkmm/scrollbar.h>
#include <gdk/gdkkeysyms.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>

#include "main.h"
#include "Settings.h"
#include "SettingsDialog.h"
#include "utils.h"

#include "ickle.h"
#include "ucompose.h"

using std::string;

SetAutoResponseDialog::SetAutoResponseDialog(Gtk::Window& parent, const string& prev_msg, bool timeout)
  : Gtk::Dialog( _("Set Auto Response"), parent)
{
  set_position(Gtk::WIN_POS_MOUSE);
  set_default_size(350,150);

  m_msg_textview.set_wrap_mode(Gtk::WRAP_WORD);
  m_msg_textview.get_buffer()->set_text(prev_msg);
  m_msg_textview.set_editable(true);

  m_msg_scr_win.add(m_msg_textview);
  m_msg_scr_win.set_size_request(0, -1); // workaround for textview horizontal resizing
  m_msg_scr_win.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
  m_msg_scr_win.set_shadow_type(Gtk::SHADOW_IN);

  build_optionmenu();
  m_autoresponse_option.signal_button_press_event().connect( SigC::slot(*this, &SetAutoResponseDialog::option_button_pressed) );

  Gtk::HButtonBox *hbox = get_action_area();

  hbox->set_border_width(0);
  hbox->pack_start(m_autoresponse_option, Gtk::PACK_EXPAND_PADDING, 0);

  // add buttons
  Gtk::Button *button;
  m_ok = button = add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );
  m_tooltip.set_tip(*button, _("Shortcuts: Ctrl+Enter or Alt-O") );
  button = add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
  m_tooltip.set_tip(*button, _("Shortcuts: Esc or Alt-C") );
  
  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  vbox->pack_start(m_msg_scr_win, Gtk::PACK_EXPAND_WIDGET);

  if (timeout)
  {
    timeout = g_settings.getValueBool("set_away_response_timeout");
  }

  m_timeout = 0;
  if (timeout)
  {
    m_timeout = 6;
    timeout_connection = Glib::signal_timeout().connect( SigC::slot( *this, &SetAutoResponseDialog::auto_timeout ), 1000 );
    auto_timeout();
  }

  set_border_width(10);
  show_all();

  m_msg_textview.grab_focus();
}

bool SetAutoResponseDialog::auto_timeout()
{
  --m_timeout;
  
  if (m_timeout == 0)
  {
    response(Gtk::RESPONSE_OK);
    return false;
  }
  else
  {
    set_title( String::ucompose( _("Set Auto Response (%1)"),
				 m_timeout ) );
    return true;
  }
}

void SetAutoResponseDialog::cancel_timeout()
{
  if (m_timeout)
  {
    timeout_connection.disconnect();
    set_title( _("Set Auto Response") );
  }
}

bool SetAutoResponseDialog::on_key_press_event(GdkEventKey* ev)
{
  if (m_timeout) cancel_timeout();

  if (ev->state & GDK_CONTROL_MASK )
  {
    if (ev->keyval == GDK_Return || ev->keyval== GDK_KP_Enter)
    {
      response(Gtk::RESPONSE_OK);
      return true;
    }
  }
  else if (ev->state & GDK_MOD1_MASK)
  {
    if (ev->keyval == GDK_o)
    { // licq shortcut
      response(Gtk::RESPONSE_OK);
      return true;
    }
  }
  if ( (ev->state & GDK_MOD1_MASK && ev->keyval == GDK_c ) ||
       ev->keyval == GDK_Escape)
  {
    response(Gtk::RESPONSE_CANCEL);
    return true;
  }

  return Gtk::Dialog::on_key_press_event(ev);
}

bool SetAutoResponseDialog::on_button_press_event(GdkEventButton *ev)
{
  if (m_timeout) cancel_timeout();
  return Gtk::Dialog::on_button_press_event(ev);
}

void SetAutoResponseDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    save_new_msg.emit(m_msg_textview.get_buffer()->get_text());
  }

  delete this;
}

void SetAutoResponseDialog::activate_menu_item_cb(int msg_index)
{
  cancel_timeout();
  
  std::string key = Utils::format_string( "autoresponse_%d_text", msg_index );

  Glib::RefPtr<Gtk::TextBuffer> buffer = m_msg_textview.get_buffer();
  buffer->erase( buffer->begin(), buffer->end() );
  buffer->insert( buffer->end(), g_settings.getValueString(key) );
}

void SetAutoResponseDialog::edit_messages_cb()
{
  cancel_timeout();
  settings_dialog.emit();
  build_optionmenu();
}

void SetAutoResponseDialog::build_optionmenu()
{
  using namespace Gtk::Menu_Helpers;
  
  /* Insert element in option menu */
  Gtk::Menu *menu = Gtk::manage( new Gtk::Menu() );
  m_autoresponse_option.set_menu( *menu );
  MenuList menu_list     = menu->items();

  int n_autoresponses = g_settings.getValueUnsignedInt("no_autoresponses");
  for (int i = 1; i <= n_autoresponses; i++)
  {
    std::string key = Utils::format_string( "autoresponse_%d_text", i );

    menu_list.push_back( MenuElem( g_settings.getValueString(key),
				   SigC::bind<int>( SigC::slot(*this,
							       &SetAutoResponseDialog::activate_menu_item_cb),
						    i) ) );
  }

  menu_list.push_back( SeparatorElem() );
  menu_list.push_back( MenuElem( _("Edit..."), SigC::slot(*this, &SetAutoResponseDialog::edit_messages_cb)) );
}

bool SetAutoResponseDialog::option_button_pressed(GdkEventButton *)
{
  cancel_timeout();
  return false;
}
