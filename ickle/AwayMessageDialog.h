/*
 * AwayMessageDialog
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

#ifndef AWAYMESSAGEDIALOG_H
#define AWAYMESSAGEDIALOG_H

#include <gtk--/window.h>
#include <gtk--/box.h>
#include <gtk--/text.h>
#include <gtk--/button.h>

#include <sigc++/signal_system.h>

#include "main.h"
#include "Settings.h"

#include <libicq2000/Contact.h>
#include <libicq2000/events.h>

class AwayMessageDialog : public Gtk::Window {
 private:
  Gtk::Window *m_main_window;
  Gtk::Text m_awaytext;
  Gtk::Button m_close_button;
  unsigned int m_pos, m_count;

  std::string format_time(time_t t);
 public:
  AwayMessageDialog(Gtk::Window *main_window);
  ~AwayMessageDialog();
  
  void messageack_cb(ICQ2000::MessageEvent *ev);
  gint button_press_cb(GdkEventButton *ev);
  void close_cb();
  gint delete_event_impl(GdkEventAny *ev);
};

#endif
