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

#include <gtkmm/box.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/table.h>
#include <gtkmm/stock.h>

#include "main.h"
#include "Settings.h"

#include "ickle.h"
#include "utils.h"

#include <libicq2000/Client.h>

using std::string;
using std::endl;

LogWindow::LogWindow()
  : m_close_button(Gtk::Stock::CLOSE), m_count(0),
    m_log_info( _("Info"), 0), m_log_warn( _("Warn"), 0), m_log_error( _("Error"), 0),
    m_log_packet( _("Packet"), 0), m_log_directpacket( _("Direct Packet"), 0)
{

  set_title( _("Log Window") );
  set_default_size(400,400);

  m_log_text.set_wrap_mode(Gtk::WRAP_WORD);
  m_log_text.set_editable(false);
  m_log_text.set_cursor_visible(false);
  
  icqclient.logger.connect( this,&LogWindow::logger_cb );
  m_close_button.signal_clicked().connect( SigC::slot(*this, &LogWindow::close_cb) );

  Gtk::VBox *box = manage( new Gtk::VBox(false) );

  // scrollbars
  m_log_scr_win.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
  m_log_scr_win.add(m_log_text);
  m_log_text.get_buffer()->create_mark("pos", m_log_text.get_buffer()->end(), true);

  Glib::RefPtr<Gtk::TextBuffer> buffer = m_log_text.get_buffer();

  m_tag_normal = buffer->create_tag("normal");

  m_tag_log_info = buffer->create_tag("log_info");
  m_tag_log_info->property_foreground().set_value( "blue" );
  m_tag_log_info->property_family().set_value( "monospace" );

  m_tag_log_warn = buffer->create_tag("log_warn");
  m_tag_log_warn->property_foreground().set_value( "red" );
  m_tag_log_warn->property_family().set_value( "monospace" );

  m_tag_log_error = buffer->create_tag("log_error");
  m_tag_log_error->property_foreground().set_value( "cyan" );
  m_tag_log_error->property_family().set_value( "monospace" );

  m_tag_log_packet = buffer->create_tag("log_packet");
  m_tag_log_packet->property_foreground().set_value( "green" );
  m_tag_log_packet->property_family().set_value( "monospace" );

  m_tag_log_directpacket = buffer->create_tag("log_directpacket");
  m_tag_log_directpacket->property_foreground().set_value( "green" );
  m_tag_log_directpacket->property_family().set_value( "monospace" );

  box->pack_start(m_log_scr_win, true, true);

  Gtk::Table *table = manage( new Gtk::Table(3, 2) );

  m_log_info.signal_toggled().connect( SigC::slot( *this, &LogWindow::checkbutton_info_cb ) );
  m_log_warn.signal_toggled().connect( SigC::slot( *this, &LogWindow::checkbutton_warn_cb ) );
  m_log_error.signal_toggled().connect( SigC::slot( *this, &LogWindow::checkbutton_error_cb ) );
  m_log_packet.signal_toggled().connect( SigC::slot( *this, &LogWindow::checkbutton_packet_cb ) );
  m_log_directpacket.signal_toggled().connect( SigC::slot( *this, &LogWindow::checkbutton_directpacket_cb ) );

  table->attach( m_log_info,         0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  table->attach( m_log_warn,         1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  table->attach( m_log_error,        2, 3, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  table->attach( m_log_packet,       0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  table->attach( m_log_directpacket, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  table->attach( m_close_button,     2, 3, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  table->set_border_width(5);

  box->pack_start(*table, Gtk::PACK_SHRINK);

  add( *box );
  box->show_all();
  realize();
}

LogWindow::~LogWindow()
{ }

void LogWindow::close_cb()
{
  hide();
}

void LogWindow::logger_cb(ICQ2000::LogEvent *ev)
{
  Glib::RefPtr<Gtk::TextBuffer> buffer = m_log_text.get_buffer();
  Glib::RefPtr<Gtk::TextBuffer::Tag> tag_log;

  switch(ev->getType())
  {
  case ICQ2000::LogEvent::INFO:
    if (!g_settings.getValueBool("log_window_info")) return;
    tag_log = m_tag_log_info;
    break;
  case ICQ2000::LogEvent::WARN:
    if (!g_settings.getValueBool("log_window_warn")) return;
    tag_log = m_tag_log_warn;
    break;
  case ICQ2000::LogEvent::ERROR:
    if (!g_settings.getValueBool("log_window_error")) return;
    tag_log = m_tag_log_error;
    break;
  case ICQ2000::LogEvent::PACKET:
    if (!g_settings.getValueBool("log_window_packet")) return;
    tag_log = m_tag_log_packet;
    break;
  case ICQ2000::LogEvent::DIRECTPACKET:
    if (!g_settings.getValueBool("log_window_directpacket")) return;
    tag_log = m_tag_log_directpacket;
    break;
  default:
    tag_log = m_tag_normal;
  }
  
  if (++m_count == 100)
  {
    Glib::RefPtr<Gtk::TextBuffer::Mark> pos = buffer->get_mark("pos");
    buffer->erase( buffer->begin(), pos->get_iter() );
    buffer->move_mark( pos, buffer->end() );
    m_count = 0;
  }

  Gtk::Adjustment *adj = m_log_scr_win.get_vadjustment();
  gfloat bot = adj->get_upper();
    
  // new-line between messages if not the first
  if (buffer->size() > 0)
    buffer->insert_with_tag( buffer->end(), "\n", m_tag_normal );

  buffer->insert_with_tag( buffer->end(), "[",           m_tag_normal );
  buffer->insert_with_tag( buffer->end(), Utils::format_time(ev->getTime()), m_tag_normal );
  buffer->insert_with_tag( buffer->end(), "]",           m_tag_normal );
  buffer->insert_with_tag( buffer->end(), ev->getMessage(), tag_log );

  adj->set_value(bot);
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

void LogWindow::on_show()
{
  // read from settings
  m_log_info.set_active( g_settings.getValueBool("log_window_info") );
  m_log_warn.set_active( g_settings.getValueBool("log_window_warn") );
  m_log_error.set_active( g_settings.getValueBool("log_window_error") );
  m_log_packet.set_active( g_settings.getValueBool("log_window_packet") );
  m_log_directpacket.set_active( g_settings.getValueBool("log_window_directpacket") );

  Gtk::Window::on_show();
}
