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

#include <gtk--/box.h>
#include <gtk--/scrollbar.h>

#include <time.h>

#include "sstream_fix.h"

#include <libicq2000/Client.h>
#include "main.h"
#include "Settings.h"

using std::string;
using std::ostringstream;
using std::endl;

using Gtk::Text_Helpers::Context;

AwayMessageDialog::AwayMessageDialog(Gtk::Window *main_window)
  : m_pos(0), m_count(0), m_main_window(main_window)
{

  set_title("Away Messages");
  set_usize(200,400);

  m_awaytext.set_word_wrap(true);
  
  icqclient.messageack.connect( slot(this,&AwayMessageDialog::messageack_cb) );

  button_release_event.connect(slot(this,&AwayMessageDialog::button_press_cb));

  Gtk::VBox *box = manage( new Gtk::VBox(false) );

  // scrollbars
  Gtk::HBox *hbox = manage( new Gtk::HBox() );
  Gtk::Scrollbar *scrollbar = manage( new Gtk::VScrollbar (*(m_awaytext.get_vadjustment())) );
  hbox->pack_start(m_awaytext, true, true, 0);
  hbox->pack_start(*scrollbar, false);
  
  box->pack_end(*hbox);

  add( *box );
  box->show_all();
  realize();
}

AwayMessageDialog::~AwayMessageDialog() { }

gint AwayMessageDialog::delete_event_impl(GdkEventAny *ev) {
  hide();
  return true;
}

gint AwayMessageDialog::button_press_cb(GdkEventButton *ev) {
  Gtk::Adjustment *adj;
  gfloat val;

  if( ev->button != 4 && ev->button != 5 ) {
    hide();
    return true;
  }
  
  adj = m_awaytext.get_vadjustment();
  val = adj->get_value();
  if( ev->button == 4 ) {
    val -= adj->get_page_increment();
  }
  else if( ev->button == 5 ) {
    val += adj->get_page_increment();
  }
  adj->set_value( val );
  return true;
}

void AwayMessageDialog::messageack_cb(ICQ2000::MessageEvent *ev) {
  if (ev->getType() != ICQ2000::MessageEvent::AwayMessage) return;
  if (!ev->isFinished()) return;

  ICQ2000::AwayMessageEvent *aev = static_cast<ICQ2000::AwayMessageEvent*>(ev);

  if (!is_visible()) {
    if (g_settings.getValueBool("away_autoposition")) {
      int width, height, x, y;
      m_main_window->get_window().get_root_origin(x, y);
      //      m_main_window->get_window().get_size(width, height);
      get_window().get_size(width, height);
      set_uposition( x - width, y );
    }
    show();
  }

  ICQ2000::ContactRef c = ev->getContact();

  m_awaytext.freeze();
  
  if (++m_count == 20) {
    m_awaytext.delete_text(0,m_pos);
    m_pos = m_awaytext.get_length();
    m_awaytext.set_point(m_pos);
    m_count = 0;
  }

  Context normal_context, header_context;
  string message_text_font = g_settings.getValueString("message_text_font");
  string message_header_font = g_settings.getValueString("message_header_font");
  if ( !message_text_font.empty() ) {
    normal_context.set_font( Gdk_Font( message_text_font ) );
  }
  if ( !message_header_font.empty() ) {
    header_context.set_font( Gdk_Font( message_header_font ) );
  }
  header_context.set_foreground( Gdk_Color("red") );
    
  Gtk::Adjustment *adj = m_awaytext.get_vadjustment();
  gfloat bot = adj->get_upper();
    
  if (m_awaytext.get_point() > 0)
    m_awaytext.insert( normal_context, "\n");

  ostringstream ostr;
  ostr << format_time( ev->getTime() ) << " "
       << c->getAlias() << endl;

  m_awaytext.insert( header_context, ostr.str());
  if (ev->isDelivered()) {
    m_awaytext.insert( normal_context, aev->getAwayMessage());
  } else {
    m_awaytext.insert( normal_context, "Couldn't fetch away message");
  }
  m_awaytext.thaw();
  adj->set_value( bot );

}

string AwayMessageDialog::format_time(time_t t) {
  struct tm *tm = localtime(&t);
  char time_str[256];
  strftime(time_str, 255, "%H:%M:%S", tm);
  return string(time_str);
}

