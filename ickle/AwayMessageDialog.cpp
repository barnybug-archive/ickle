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

#include "AwayMessageDialog.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/stock.h>

#include <time.h>

#include "sstream_fix.h"

#include <libicq2000/Client.h>
#include "main.h"
#include "Settings.h"

using std::string;
using std::ostringstream;
using std::endl;

AwayMessageDialog::AwayMessageDialog(Gtk::Window& main_window)
  : m_main_window(main_window), m_count(0)
{

  set_title("Away Messages");
  set_default_size(200,400);

  m_away_textview.set_wrap_mode(Gtk::WRAP_WORD);
  
  icqclient.messageack.connect( this, &AwayMessageDialog::messageack_cb );

  signal_button_release_event().connect( SigC::slot(*this,&AwayMessageDialog::button_press_cb) );
  
  Gtk::Button *close_button = manage( new Gtk::Button( Gtk::Stock::CLOSE ) );
  close_button->signal_clicked().connect( SigC::slot(*this, &AwayMessageDialog::close_cb) );

  Gtk::VBox *box = manage( new Gtk::VBox(false) );

  m_away_scr_win.add(m_away_textview);
  m_away_scr_win.set_size_request(0, -1); // workaround for textview horizontal resizing
  m_away_scr_win.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);

  box->pack_start(m_away_scr_win, Gtk::PACK_EXPAND_WIDGET);

  m_away_textview.get_buffer()->create_mark("pos", m_away_textview.get_buffer()->end(), true);
  m_away_textview.set_editable(false);
  m_away_textview.set_cursor_visible(false);

  box->pack_start(*close_button, Gtk::PACK_SHRINK);

  Glib::RefPtr<Gtk::TextBuffer> buffer = m_away_textview.get_buffer();
  m_tag_header = buffer->create_tag("heading");
  m_tag_normal = buffer->create_tag("normal");
  
  string message_text_font = g_settings.getValueString("message_text_font");
  string message_header_font = g_settings.getValueString("message_header_font");
  if ( !message_text_font.empty() )
  {
    m_tag_normal->property_font().set_value( message_text_font );
  }
  if ( !message_header_font.empty() )
  {
    m_tag_header->property_font().set_value( message_header_font );
  }
  m_tag_header->property_foreground().set_value( "red" );
    
  add( *box );
  box->show_all();
  realize();
}

AwayMessageDialog::~AwayMessageDialog()
{ }

bool AwayMessageDialog::button_press_cb(GdkEventButton *ev)
{
  Gtk::Adjustment *adj;
  gfloat val;

  if ( ev->button != 4 && ev->button != 5 )
  {
    hide();
    return true;
  }

  /* mouse scroll-wheel shenanigans */

  adj = m_away_scr_win.get_vadjustment();
  val = adj->get_value();

  if( ev->button == 4 )
  {
    val -= adj->get_page_increment();
  }
  else if( ev->button == 5 )
  {
    val += adj->get_page_increment();
  }

  adj->set_value( val );

  return true;
}

void AwayMessageDialog::close_cb()
{
  hide();
}

void AwayMessageDialog::messageack_cb(ICQ2000::MessageEvent *ev)
{
  if (ev->getType() != ICQ2000::MessageEvent::AwayMessage)
    return;

  if (!ev->isFinished())
    return;

  ICQ2000::AwayMessageEvent *aev = static_cast<ICQ2000::AwayMessageEvent*>(ev);

  if (!is_visible())
  {
    if (g_settings.getValueBool("away_autoposition"))
    {
      int width, height, x, y;
      m_main_window.get_window()->get_root_origin(x, y);
      get_window()->get_size(width, height);
      move( x - width, y );
    }

    show();
  }

  ICQ2000::ContactRef c = ev->getContact();

  Glib::RefPtr<Gtk::TextBuffer> buffer = m_away_textview.get_buffer();

  if (++m_count == 20)
  {
    Glib::RefPtr<Gtk::TextBuffer::Mark> pos = buffer->get_mark("pos");
    buffer->erase( buffer->begin(), pos->get_iter() );
    buffer->move_mark( pos, buffer->end() );
    m_count = 0;
  }

  Gtk::Adjustment *adj = m_away_scr_win.get_vadjustment();
  gfloat bot = adj->get_upper();
    
  // new-line between messages if not the first
  if (buffer->size() > 0)
    buffer->insert_with_tag( buffer->end(), "\n", m_tag_normal );

  ostringstream ostr;
  ostr << format_time( ev->getTime() ) << " "
       << c->getAlias() << endl;

  buffer->insert_with_tag( buffer->end(), ostr.str(), m_tag_header);

  if (ev->isDelivered())
  {
    buffer->insert_with_tag( buffer->end(), aev->getAwayMessage(), m_tag_normal);
  }
  else
  {
    buffer->insert_with_tag( buffer->end(), "Couldn't fetch away message", m_tag_normal);
  }

  adj->set_value( bot );
}

string AwayMessageDialog::format_time(time_t t)
{
  struct tm *tm = localtime(&t);
  char time_str[256];
  strftime(time_str, 255, "%H:%M:%S", tm);
  return string(time_str);
}

