/*
 * LogWindow
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#include "LogWindow.h"

#include <gtk--/box.h>
#include <gtk--/scrollbar.h>
#include <gtk--/table.h>

#include <time.h>

#include "sstream_fix.h"

#include "main.h"
#include "Settings.h"

#include <libicq2000/Client.h>

using std::string;
using std::ostringstream;
using std::endl;

using Gtk::Text_Helpers::Context;

LogWindow::LogWindow(Gtk::Window *main_window)
  : m_main_window(main_window),
    m_close_button("Close"),
    m_pos(0), m_count(0),
    m_log_info("Info", 0), m_log_warn("Warn", 0), m_log_error("Error", 0),
    m_log_packet("Packet", 0), m_log_directpacket("Direct Packet", 0)
{

  set_title("Log Window");
  set_usize(400,400);

  m_log_text.set_word_wrap(true);
  
  icqclient.logger.connect( slot(this,&LogWindow::logger_cb) );
  m_close_button.clicked.connect( slot(this, &LogWindow::close_cb) );

  Gtk::VBox *box = manage( new Gtk::VBox(false) );

  // scrollbars
  Gtk::HBox *hbox = manage( new Gtk::HBox() );
  Gtk::Scrollbar *scrollbar = manage( new Gtk::VScrollbar (*(m_log_text.get_vadjustment())) );
  hbox->pack_start(m_log_text, true, true, 0);
  hbox->pack_start(*scrollbar, false);
  
  box->pack_start(*hbox, true, true);

  Gtk::Table *table = manage( new Gtk::Table(3, 2) );

  m_log_info.toggled.connect( slot( this, &LogWindow::checkbutton_info_cb ) );
  m_log_warn.toggled.connect( slot( this, &LogWindow::checkbutton_warn_cb ) );
  m_log_error.toggled.connect( slot( this, &LogWindow::checkbutton_error_cb ) );
  m_log_packet.toggled.connect( slot( this, &LogWindow::checkbutton_packet_cb ) );
  m_log_directpacket.toggled.connect( slot( this, &LogWindow::checkbutton_directpacket_cb ) );

  table->attach( m_log_info,         0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );
  table->attach( m_log_warn,         1, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );
  table->attach( m_log_error,        2, 3, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );
  table->attach( m_log_packet,       0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );
  table->attach( m_log_directpacket, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );
  table->attach( m_close_button,     2, 3, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND );
  table->set_border_width(5);

  box->pack_start(*table, false);

  add( *box );
  box->show_all();
  realize();
}

LogWindow::~LogWindow()
{ }

gint LogWindow::delete_event_impl(GdkEventAny *)
{
  hide();
  return true;
}

void LogWindow::close_cb()
{
  hide();
}

void LogWindow::logger_cb(ICQ2000::LogEvent *ev)
{
  Context normal_context, log_context;

  switch(ev->getType()) {
  case ICQ2000::LogEvent::INFO:
    if (!g_settings.getValueBool("log_window_info")) return;
    log_context.set_foreground( Gdk_Color("blue") );
    break;
  case ICQ2000::LogEvent::ERROR:
    if (!g_settings.getValueBool("log_window_error")) return;
    log_context.set_foreground( Gdk_Color("red") );
    break;
  case ICQ2000::LogEvent::WARN:
    if (!g_settings.getValueBool("log_window_warn")) return;
    log_context.set_foreground( Gdk_Color("cyan") );
    break;
  case ICQ2000::LogEvent::PACKET:
    if (!g_settings.getValueBool("log_window_packet")) return;
    log_context.set_foreground( Gdk_Color("green") );
    break;
  case ICQ2000::LogEvent::DIRECTPACKET:
    if (!g_settings.getValueBool("log_window_directpacket")) return;
    log_context.set_foreground( Gdk_Color("green") );
    break;
  }
  
  m_log_text.freeze();

  if (++m_count == 100) {
    m_log_text.delete_text(0,m_pos);
    m_pos = m_log_text.get_length();
    m_log_text.set_point(m_pos);
    m_count = 0;
  }

  Gtk::Adjustment *adj = m_log_text.get_vadjustment();
  gfloat bot = adj->get_upper();
    
  if (m_log_text.get_point() > 0)
    m_log_text.insert( normal_context, "\n" );

  ostringstream ostr;
  ostr << "[" << format_time( ev->getTime() ) << "] ";
  m_log_text.insert( normal_context, ostr.str() );
  m_log_text.insert( log_context, ev->getMessage() );

  m_log_text.thaw();
  adj->set_value(bot);
}

string LogWindow::format_time(time_t t)
{
  struct tm *tm = localtime(&t);
  char time_str[256];
  strftime(time_str, 255, "%H:%M:%S", tm);
  return string(time_str);
}

void LogWindow::checkbutton_info_cb()
{
  g_settings.setValue( "log_window_info", m_log_info.get_active() );
}

void LogWindow::checkbutton_warn_cb()
{
  g_settings.setValue( "log_window_warn", m_log_warn.get_active() );
}

void LogWindow::checkbutton_error_cb()
{
  g_settings.setValue( "log_window_error", m_log_error.get_active() );
}

void LogWindow::checkbutton_packet_cb()
{
  g_settings.setValue( "log_window_packet", m_log_packet.get_active() );
}

void LogWindow::checkbutton_directpacket_cb()
{
  g_settings.setValue( "log_window_directpacket", m_log_directpacket.get_active() );
}

void LogWindow::show_impl()
{
  // read from settings
  m_log_info.set_active( g_settings.getValueBool("log_window_info") );
  m_log_warn.set_active( g_settings.getValueBool("log_window_warn") );
  m_log_error.set_active( g_settings.getValueBool("log_window_error") );
  m_log_packet.set_active( g_settings.getValueBool("log_window_packet") );
  m_log_directpacket.set_active( g_settings.getValueBool("log_window_directpacket") );

  Gtk::Window::show_impl();
}
