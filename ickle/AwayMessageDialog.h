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

#include <gtkmm/window.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/signal.h>

#include "main.h"
#include "Settings.h"

#include <libicq2000/Contact.h>
#include <libicq2000/events.h>

class AwayMessageDialog : public Gtk::Window,
			  public sigslot::has_slots<>
{
 private:
  Gtk::Window& m_main_window;

  Gtk::ScrolledWindow m_away_scr_win;
  Gtk::TextView m_away_textview;
  Glib::RefPtr<Gtk::TextTag> m_tag_header, m_tag_normal;
  unsigned int m_count;

 public:
  AwayMessageDialog(Gtk::Window& main_window);
  ~AwayMessageDialog();
  
  void messageack_cb(ICQ2000::MessageEvent *ev);
  bool button_press_cb(GdkEventButton *ev);
  void close_cb();
};

#endif
