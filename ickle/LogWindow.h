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

#include <gtkmm/window.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>

#include <sigc++/signal.h>

#include <libicq2000/events.h>

class LogWindow : public Gtk::Window,
		  public sigslot::has_slots<>
{
 private:
  Gtk::Button m_close_button;
  unsigned int m_count;
  Gtk::CheckButton m_log_info, m_log_warn, m_log_error, m_log_packet, m_log_directpacket;
  Gtk::ScrolledWindow m_log_scr_win;
  Gtk::TextView m_log_text;

  Glib::RefPtr<Gtk::TextTag> m_tag_log_info, m_tag_log_warn, m_tag_log_error,
                                     m_tag_log_packet, m_tag_log_directpacket, m_tag_normal;

  std::string format_time(time_t t);

  void checkbutton_info_cb();
  void checkbutton_warn_cb();
  void checkbutton_error_cb();
  void checkbutton_packet_cb();
  void checkbutton_directpacket_cb();

 protected:
  virtual void on_show();
  
 public:
  LogWindow();
  ~LogWindow();
  
  void logger_cb(ICQ2000::LogEvent *ev);
  void close_cb();
  gint delete_event_impl(GdkEventAny *ev);
};

#endif
