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

#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/main.h>

PromptDialog::PromptDialog(PromptType t, const string& msg)
  : Gtk::Dialog(),
    type(t),
    finish_bool(false)
{
  set_modal(true);
  set_position(GTK_WIN_POS_MOUSE);

  destroy.connect( Gtk::Main::quit.slot() );

  Gtk::HBox *hbox = get_action_area();
  Gtk::Button *button;

  switch(t) {
  case PROMPT_INFO:
    set_title("ickle information");
    button = manage( new Gtk::Button("OK") );
    button->set_usize(70,20);
    button->clicked.connect( slot( this, &PromptDialog::true_cb ) );
    hbox->pack_start(*button, true, false);
    break;
  case PROMPT_WARNING:
    set_title("ickle warning");
    button = manage( new Gtk::Button("OK") );
    button->set_usize(70,20);
    button->clicked.connect( slot( this, &PromptDialog::true_cb ) );
    hbox->pack_start(*button, true, false);
    break;
  case PROMPT_CONFIRM:
    set_title("ickle confirmation");
    button = manage( new Gtk::Button("OK") );
    button->set_usize(70,20);
    button->clicked.connect( slot( this, &PromptDialog::true_cb ) );
    hbox->pack_start(*button, true, false, 0);
    button = manage( new Gtk::Button("Cancel") );
    button->set_usize(70,20);
    button->clicked.connect( slot( this, &PromptDialog::false_cb ) );
    hbox->pack_start(*button, true, false, 0);
    break;
  case PROMPT_QUESTION:
    set_title("ickle question");
    button = manage( new Gtk::Button("Yes") );
    button->set_usize(70,20);
    button->clicked.connect( slot( this, &PromptDialog::true_cb ) );
    hbox->pack_start(*button, true, false, 0);
    button = manage( new Gtk::Button("No") );
    button->set_usize(70,20);
    button->clicked.connect( slot( this, &PromptDialog::false_cb ) );
    hbox->pack_start(*button, true, false, 0);
    break;
  }

  Gtk::Label *label = manage( new Gtk::Label( msg, 0 ) );
  label->set_line_wrap(true);
  Gtk::VBox *vbox = get_vbox();
  vbox->pack_start( *label, true, true );

  set_border_width(10);
  //  set_usize(300,150);
  show_all();
}

bool PromptDialog::run() {
  Gtk::Main::run();
  return finish_bool;
}

void PromptDialog::true_cb() {
  finish_bool = true;
  destroy.emit();
}

void PromptDialog::false_cb() {
  finish_bool = false;
  destroy.emit();
}
