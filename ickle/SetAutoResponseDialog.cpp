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

#include <gtk--/table.h>
#include <gtk--/scrollbar.h>
#include <gdk/gdkkeysyms.h>
#include <gtk--/menu.h>

#include "main.h"
#include "Settings.h"
#include "sstream_fix.h"
#include "SettingsDialog.h"

using std::ostringstream;

SetAutoResponseDialog::SetAutoResponseDialog(Gtk::Window * parent, const string& prev_msg, bool timeout)
  : Gtk::Dialog(),
    okay("OK"), cancel("Cancel")
{
  set_title("Set Auto Response");
  set_position(GTK_WIN_POS_MOUSE);
  set_transient_for (*parent);
  set_usize(350,150);

  msg_input.set_word_wrap(true);
  msg_input.insert(prev_msg);
  msg_input.set_editable(true);

  okay.clicked.connect(slot(this,&SetAutoResponseDialog::okay_cb));
  cancel.clicked.connect(slot(this,&SetAutoResponseDialog::cancel_cb));

  build_optionmenu();
  autoresponse_option.button_press_event.connect( slot(this, &SetAutoResponseDialog::option_button_pressed) );

  Gtk::HBox *hbox = get_action_area();
  hbox->set_border_width(0);
  hbox->pack_start(autoresponse_option, true, true, 0);
  hbox->pack_start(okay, true, true, 0);
  hbox->pack_start(cancel, true, true, 0);
  m_tooltip.set_tip(okay, "Shortcuts: Ctrl+Enter or Alt-O");
  m_tooltip.set_tip(cancel, "Shortcuts: Esc or Alt-C");

  Gtk::Table *table = manage( new Gtk::Table(2,1,false) );
  table->attach( msg_input, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,
		 GTK_FILL | GTK_EXPAND | GTK_SHRINK);
  Gtk::Scrollbar *scrollbar = manage( new Gtk::VScrollbar (*(msg_input.get_vadjustment())) );
  table->attach (*scrollbar, 1, 2, 0, 1, 0, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  vbox->pack_start(*table, true, true);

  if (timeout) {
    timeout = g_settings.getValueBool("set_away_response_timeout");
  }

  m_timeout = 0;
  if (timeout) {
    m_timeout = 5;
    Gtk::Label *label = static_cast<Gtk::Label*>(okay.get_child());
    label->set_text("OK (5)");
    timeout_connection = Gtk::Main::timeout.connect( slot( this, &SetAutoResponseDialog::auto_timeout ), 1000 );
  }

  set_border_width(10);
  show_all();
  msg_input.grab_focus();
}

int SetAutoResponseDialog::auto_timeout() {
  --m_timeout;
  if (m_timeout == 0) {
    okay.clicked();
    return false;
  } else {
    std::ostringstream ostr;
    ostr << "OK (" << m_timeout << ")";
    Gtk::Label *label = static_cast<Gtk::Label*>(okay.get_child());
    label->set_text(ostr.str());
    return true;
  }
}

void SetAutoResponseDialog::cancel_timeout()
{
  if (m_timeout) {
    timeout_connection.disconnect();
    Gtk::Label *label = static_cast<Gtk::Label*>(okay.get_child());
    label->set_text("OK");
  }
}

gint SetAutoResponseDialog::key_press_event_impl(GdkEventKey* ev) {
  if (m_timeout) cancel_timeout();

  if (ev->state & GDK_CONTROL_MASK ) {
    if (ev->keyval == GDK_Return || ev->keyval== GDK_KP_Enter)
      okay.clicked();
  } else if (ev->state & GDK_MOD1_MASK) {
    if (ev->keyval == GDK_o) // licq shortcut
      okay.clicked();
  }
  if ( (ev->state & GDK_MOD1_MASK && ev->keyval == GDK_c ) ||
       ev->keyval == GDK_Escape)
    cancel.clicked();

  return Gtk::Dialog::key_press_event_impl(ev);
}

gint SetAutoResponseDialog::button_press_event_impl(GdkEventButton *ev)
{
  if (m_timeout) cancel_timeout();
  return Gtk::Dialog::button_press_event_impl(ev);
}

void SetAutoResponseDialog::okay_cb() {
  save_new_msg.emit(msg_input.get_chars(0, -1));
  destroy.emit();
}

void SetAutoResponseDialog::cancel_cb() {
  destroy.emit();
}

void SetAutoResponseDialog::activate_menu_item_cb(int msg_index) {
  cancel_timeout();
  ostringstream fetch_str;
  fetch_str << "autoresponse_" << msg_index << "_text";
  msg_input.delete_text(0,-1);
  msg_input.insert( g_settings.getValueString(fetch_str.str()) );  
}

void SetAutoResponseDialog::edit_messages_cb() {
  cancel_timeout();
  settings_dialog.emit();
  build_optionmenu();
}

void SetAutoResponseDialog::build_optionmenu()
{
  using namespace Gtk::Menu_Helpers;
  using SigC::bind;
  /* Insert element in option menu */
  Gtk::Menu *menu = manage( new Gtk::Menu() );
  MenuList& menu_list     = menu->items();
  int n_autoresponses = g_settings.getValueUnsignedInt("no_autoresponses");
  for (int i = 1; i <= n_autoresponses; i++) {
    ostringstream fetch_str;
    fetch_str << "autoresponse_" << i << "_label";
    menu_list.push_back( MenuElem( g_settings.getValueString(fetch_str.str()),
				   bind<int>( slot(this, 
						   &SetAutoResponseDialog::activate_menu_item_cb), i) )
			 );
  }
  menu_list.push_back( SeparatorElem() );
  menu_list.push_back( MenuElem("Edit...", slot(this, &SetAutoResponseDialog::edit_messages_cb)) );
  autoresponse_option.set_menu( menu );
}

gint SetAutoResponseDialog::option_button_pressed(GdkEventButton *b) {
  cancel_timeout();
}
