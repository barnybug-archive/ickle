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

#include "main.h"
#include "Settings.h"
#include "sstream_fix.h"

using std::ostringstream;

SetAutoResponseDialog::SetAutoResponseDialog(const string& prev_msg)
  : Gtk::Dialog(),
    okay("OK"), cancel("Cancel")
{
  set_title("Set Auto Response");
  set_position(GTK_WIN_POS_MOUSE);
  set_usize(300,150);

  msg_input.set_word_wrap(true);
  msg_input.insert(prev_msg);
  msg_input.set_editable(true);

  okay.clicked.connect(slot(this,&SetAutoResponseDialog::okay_cb));
  cancel.clicked.connect(slot(this,&SetAutoResponseDialog::cancel_cb));

  Gtk::HBox *hbox = get_action_area();
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
  vbox->pack_start(*table, true, true, 5);

  bool timeout = g_settings.getValueBool("set_away_response_timeout");
  m_timeout = 0;
  if (timeout) {
    m_timeout = 5;
    Gtk::Label *label = static_cast<Gtk::Label*>(okay.get_child());
    label->set_text("OK (5)");
    timeout_connection = Gtk::Main::timeout.connect( slot( this, &SetAutoResponseDialog::auto_timeout ), 1000 );
  }

  set_border_width(5);
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
