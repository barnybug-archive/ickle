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

using std::ostringstream;
using std::endl;

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
  hide();
  return true;
}

void AwayMessageDialog::messageack_cb(MessageEvent *ev) {
  if (ev->getType() != MessageEvent::AwayMessage) return;
  if (!ev->isFinished()) return;

  AwayMessageEvent *aev = static_cast<AwayMessageEvent*>(ev);

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

  Contact *c = ev->getContact();

  if (++m_count == 20) {
    m_awaytext.delete_text(0,m_pos);
    m_pos = m_awaytext.get_length();
    m_awaytext.set_point(m_pos);
    m_count = 0;
  }

  Gdk_Font normal_font;
  Gdk_Font header_font;
  string message_text_font = g_settings.getValueString("message_text_font");
  string message_header_font = g_settings.getValueString("message_header_font");
  if ( !message_text_font.empty() ) {
    normal_font = Gdk_Font( message_text_font );
  }
  if ( !message_header_font.empty() ) {
    header_font = Gdk_Font( message_header_font );
  }
  Gdk_Color nickc("red");
  Gdk_Color white("white");
  Gdk_Color black("black");
    
  Gtk::Adjustment *adj = m_awaytext.get_vadjustment();
  gfloat bot = adj->get_upper();
    
  if (m_awaytext.get_point() > 0)
    m_awaytext.insert( normal_font, black, white, "\n", -1);

  ostringstream ostr;
  ostr << format_time( ev->getTime() ) << " "
       << c->getAlias() << endl;
  m_awaytext.freeze();
  m_awaytext.insert( header_font, nickc, white, ostr.str(), -1);
  if (ev->isDelivered()) {
    m_awaytext.insert( normal_font, black, white, aev->getMessage(), -1);
  } else {
    m_awaytext.insert( normal_font, black, white, "Couldn't fetch away message", -1);
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

