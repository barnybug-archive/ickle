/*
 * LogWindow
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

#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <gtk--/window.h>
#include <gtk--/text.h>
#include <gtk--/button.h>
#include <gtk--/checkbutton.h>

#include <sigc++/signal_system.h>

#include <libicq2000/events.h>

class LogWindow : public Gtk::Window {
 private:
  Gtk::Window *m_main_window;
  Gtk::Text m_log_text;
  Gtk::Button m_close_button;
  Gtk::CheckButton m_log_info, m_log_error, m_log_warn, m_log_packet, m_log_directpacket;
  unsigned int m_pos, m_count;

  std::string format_time(time_t t);

  void checkbutton_info_cb();
  void checkbutton_warn_cb();
  void checkbutton_error_cb();
  void checkbutton_packet_cb();
  void checkbutton_directpacket_cb();

 protected:
  virtual void show_impl();
  
 public:
  LogWindow(Gtk::Window *main_window);
  ~LogWindow();
  
  void logger_cb(ICQ2000::LogEvent *ev);
  void close_cb();
  gint delete_event_impl(GdkEventAny *ev);
};

#endif
