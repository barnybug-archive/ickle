/*
 * MessageBox
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

#include "MessageBox.h"

#include "sstream_fix.h"

#include <gtk--/imageloader.h>
#include <gtk--/pixmap.h>
#include <gtk--/scrollbar.h>
#include <gdk/gdkkeysyms.h>

using std::ostringstream;
using std::endl;

MessageBox::MessageBox(Contact *c)
  : m_contact(c),
    m_send_button("Send"), m_close_button("Close"),
    m_vbox_top(false,10),
    m_history_table(2,1,false),
    m_sms_count_label("", 0),
    m_sms_count_over(false),
    m_sms_enabled(true)
{
  set_contact_title();
  set_border_width(10);
  //  set_usize(450,300);

  m_pane.set_handle_size (8);
  m_pane.set_gutter_size (12);                       
  
  // -- top pane --

  Gtk::Scrollbar *scrollbar;
  
  m_history_table.set_usize(400,100);
  m_history_table.attach(m_history_text, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,
			 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);

  // scrollbars
  scrollbar = manage( new Gtk::VScrollbar (*(m_history_text.get_vadjustment())) );
  m_history_table.attach (*scrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
  
  m_history_text.set_editable(false);
  m_history_text.set_word_wrap(true);

  m_pane.add1(m_history_table);

  // -- bottom pane --

  Gtk::ImageLoader *l;
  Gtk::Pixmap *i;
  Gtk::Table *table;

  // tab index

  m_tab.set_tab_pos(GTK_POS_LEFT);

  if ( c->isICQContact() ) {
    m_message_type = MessageEvent::Normal;
    m_tab.switch_page.connect(slot(this,&MessageBox::switch_page_cb));

  // -- normal message tab --
    table = manage( new Gtk::Table( 2, 1, false ) );
    table->set_usize(400,100);
    table->attach(m_message_text, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,
		  GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
    scrollbar = manage( new Gtk::VScrollbar (*(m_message_text.get_vadjustment())) );
    table->attach (*scrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
  
    m_message_text.set_word_wrap(true);
    m_message_text.set_editable(true);
    m_message_text.key_press_event.connect(slot(this,&MessageBox::key_press_cb));

    l = g_icons.IconForEvent(MessageEvent::Normal);
    i = manage( new Gtk::Pixmap( l->pix(), l->bit() ) );

    m_tab.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *table, *i )  );
    // -------------------------

  // -------- url tab --------
    Gtk::Box *url_hbox = manage( new Gtk::HBox() );
    Gtk::Box *url_vbox = manage( new Gtk::VBox() );
    table = manage( new Gtk::Table( 2, 1, false ) );
    table->set_usize(400,100);
    table->attach(m_url_text, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,
		  GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
    scrollbar = manage( new Gtk::VScrollbar (*(m_url_text.get_vadjustment())) );
    table->attach (*scrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
  
    m_url_text.set_word_wrap(true);
    m_url_text.set_editable(true);
    m_url_text.key_press_event.connect(slot(this,&MessageBox::key_press_cb));

    url_vbox->pack_start( *table, true, true );

    url_hbox->pack_start( * manage( new Gtk::Label("URL:") ), false );
    url_hbox->pack_start( m_url_entry, true, true );

    url_vbox->pack_start( *url_hbox, false );

    l = g_icons.IconForEvent(MessageEvent::URL);
    i = manage( new Gtk::Pixmap( l->pix(), l->bit() ) );

    m_tab.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *url_vbox, *i )  );
    // -------------------------
  } else {
    m_message_type = MessageEvent::SMS;
  }

  // -------- sms tab --------
  Gtk::Box *sms_hbox = manage( new Gtk::HBox() );
  Gtk::Box *sms_vbox = manage( new Gtk::VBox() );
  table = manage( new Gtk::Table( 2, 1, false ) );
  table->set_usize(400,100);
  table->attach(m_sms_text, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,
		GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
  scrollbar = manage( new Gtk::VScrollbar (*(m_sms_text.get_vadjustment())) );
  table->attach (*scrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
  
  m_sms_text.set_word_wrap(true);
  m_sms_text.set_editable(true);
  m_sms_text.changed.connect( slot( this, &MessageBox::sms_count_update_cb ) );
  m_sms_text.key_press_event.connect(slot(this,&MessageBox::key_press_cb));
  if (!c->isMobileContact()) disable_sms();
  sms_count_update_cb();

  sms_vbox->pack_start( *table, true, true );

  m_sms_count.set_editable(false);
  m_sms_count.set_usize(40,-1);
  sms_hbox->pack_start( m_sms_count, false );

  m_sms_count_label.set_justify(GTK_JUSTIFY_LEFT);
  sms_hbox->pack_start( m_sms_count_label, true, true );

  sms_vbox->pack_start( *sms_hbox, false );

  l = g_icons.IconForEvent(MessageEvent::SMS);
  i = manage( new Gtk::Pixmap( l->pix(), l->bit() ) );

  m_tab.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *sms_vbox, *i )  );
  // -------------------------

  m_pane.add2(m_tab);

  m_vbox_top.pack_start(m_pane,true,true);

  // -- button bar --

  m_send_button.clicked.connect(slot(this,&MessageBox::send_clicked_cb));
  m_close_button.clicked.connect( destroy.slot() );

  m_hbox_buttons.pack_start(m_send_button);
  m_hbox_buttons.pack_end(m_close_button);

  m_vbox_top.pack_start(m_hbox_buttons,false);

  m_vbox_top.pack_start(m_status, TRUE, TRUE, 0);
  m_status_context = m_status.get_context_id("messagebox");

  add(m_vbox_top);

  show_all();
}

MessageBox::~MessageBox() { }

void MessageBox::raise() const {
  get_window().show();
}

gint MessageBox::key_press_cb(GdkEventKey* ev) {
  if (m_online) {
    if (ev->state & GDK_CONTROL_MASK ) {
      if (ev->keyval == GDK_Return || ev->keyval== GDK_KP_Enter)
        m_send_button.clicked();
    } else if (ev->state & GDK_MOD1_MASK) {
      if (ev->keyval == GDK_s)
        m_send_button.clicked();
    }
  }

  if ( (ev->state & GDK_MOD1_MASK && ev->keyval == GDK_c ) ||
       ev->keyval == GDK_Escape)
    destroy.emit();

  return false;
}

void MessageBox::set_contact_title() {
  ostringstream ostr;
  ostr << m_contact->getAlias() << " (";
  if (m_contact->isICQContact()) {
    ostr << m_contact->getUIN();
  } else {
    ostr << m_contact->getMobileNo();
  }
  ostr << ")";
  set_title(ostr.str());
}

void MessageBox::contactlist_cb(ContactListEvent *ev) {
  if (m_contact->isMobileContact()) enable_sms();
  else disable_sms();

  // in case Alias has changed
  set_contact_title();
}

void MessageBox::enable_sms() {
  m_sms_text.set_sensitive(true);
  m_sms_count.set_sensitive(true);
  m_sms_count_label.set_sensitive(true);
  m_sms_enabled = true;
  send_button_update();
}

void MessageBox::disable_sms() {
  m_sms_text.set_sensitive(false);
  m_sms_count.set_sensitive(false);
  m_sms_count_label.set_sensitive(false);
  m_sms_enabled = false;
  send_button_update();
}

void MessageBox::online() {
  m_online = true;
  send_button_update();
}

void MessageBox::offline() {
  m_online = false;
  send_button_update();
}

void MessageBox::setDisplayTimes(bool d) {
  m_display_times = d;
}

void MessageBox::send_button_update() {
  if (m_message_type == MessageEvent::SMS) {
    if (m_sms_enabled && m_online && !m_sms_count_over) m_send_button.set_sensitive(true);
    else m_send_button.set_sensitive(false);
  } else {
    if (m_online) m_send_button.set_sensitive(true);
    else m_send_button.set_sensitive(false);
  }
}

void MessageBox::sms_count_update_cb() {
  guint len = m_sms_text.get_length();
  ostringstream ostr;
  if (len > SMS_Max_Length) {
    m_sms_count_over = true;
    ostr << (len - SMS_Max_Length);
    m_sms_count_label.set_text("chars over");
    send_button_update();
  } else {
    m_sms_count_over = false;
    ostr << (SMS_Max_Length - len);
    m_sms_count_label.set_text("chars left");
    send_button_update();
  }
  m_sms_count.set_text(ostr.str());
}

void MessageBox::switch_page_cb(Gtk::Notebook_Helpers::Page* p, guint n) {
  if (n == 0 && m_contact->isICQContact() ) {
    m_message_type = MessageEvent::Normal;
    m_message_text.grab_focus();
  } else if ( n == 1 ) {
    m_message_type = MessageEvent::URL;
    m_url_text.grab_focus();
  } else if ( n == 2 || ( n == 0 && !m_contact->isICQContact() ) ) {
    m_message_type = MessageEvent::SMS;
    m_sms_text.grab_focus();
    sms_count_update_cb();
  }
  send_button_update();
}

void MessageBox::messageack_cb(MessageEvent *ev) {
  Contact *c = ev->getContact();
  if (c->getUIN() != m_contact->getUIN()) return;

  if (ev->isFinished()) {
    if (ev->isDelivered()) {
      display_message(ev, false, "You");
      
      if (m_message_type == MessageEvent::Normal) {
	m_message_text.delete_text(0,-1);
      } else if (m_message_type == MessageEvent::URL) {
	m_url_entry.delete_text(0,-1);
	m_url_text.delete_text(0,-1);
      } else if (m_message_type == MessageEvent::SMS) {
	m_sms_text.delete_text(0,-1);
      }
      
      set_status("Sent message successfully");
    } else {
      set_status("Sending message timed out");
    }
  }


}

void MessageBox::display_message(MessageEvent *ev, bool sent, const string& nick) {

  Gdk_Font normal_font;
  Gdk_Font bold_font("-*-*-bold-*-*-*-*-*-*-*-*-*-*-*");

  Gdk_Color nickc;
  if (sent) nickc = Gdk_Color("red");
  else nickc = Gdk_Color("blue");

  Gdk_Color white("white");
  Gdk_Color black("black");
  
  m_history_text.freeze();

  Gtk::Adjustment *adj = m_history_text.get_vadjustment();
  gfloat bot = adj->get_upper();

  m_history_text.insert( normal_font, black, white, "\n", -1);

  ostringstream ostr;
  if (m_display_times) {
    time_t t = ev->getTime();
    ostr << format_time(t) << " ";
  }

  ostr << nick << " ";
  if (ev->getType() == MessageEvent::Normal) {
    NormalMessageEvent *msg = static_cast<NormalMessageEvent*>(ev);

    if ( msg->isMultiParty() ) ostr << "[multiparty] ";
    m_history_text.insert( bold_font, nickc, white, ostr.str(), -1);
    m_history_text.insert( normal_font, black, white, msg->getMessage(), -1);
      
  } else if (ev->getType() == MessageEvent::URL) {
    URLMessageEvent *url = static_cast<URLMessageEvent*>(ev);

    m_history_text.insert( bold_font, nickc, white, ostr.str(), -1);
    m_history_text.insert( normal_font, black, white, url->getURL(), -1);
    m_history_text.insert( normal_font, black, white, "\n", -1);
    m_history_text.insert( normal_font, black, white, url->getMessage(), -1);
      
  } else if (ev->getType() == MessageEvent::SMS) {
    SMSMessageEvent *smsmsg = static_cast<SMSMessageEvent*>(ev);

    ostr << "[sms] ";
    m_history_text.insert( bold_font, nickc, white, ostr.str(), -1);
    m_history_text.insert( normal_font, black, white, smsmsg->getMessage(), -1);
      
  } else if (ev->getType() == MessageEvent::SMS_Response) {
    SMSResponseEvent *rsp = static_cast<SMSResponseEvent*>(ev);
    if (rsp->deliverable()) {
    } else {
    }
      
  } else if (ev->getType() == MessageEvent::SMS_Receipt) {
    SMSReceiptEvent *rcpt = static_cast<SMSReceiptEvent*>(ev);

    if (rcpt->delivered()) {
      ostr << "[sms delivered]";
    } else {
      ostr << "[sms not delivered]";
    }
    ostr << endl;
    m_history_text.insert( bold_font, nickc, white, ostr.str(), -1);

  }

  m_history_text.thaw();
  adj->set_value( bot );
}

bool MessageBox::message_cb(MessageEvent *ev) {
  Contact *c = ev->getContact();
  if (c->getUIN() != m_contact->getUIN()) return false;
  display_message(ev, true, m_contact->getAlias());
  return true;
}
  
void MessageBox::send_clicked_cb() {

  set_status("Sending message...");

  if (m_message_type == MessageEvent::Normal) {
    NormalMessageEvent *nv = new NormalMessageEvent( m_contact, m_message_text.get_chars(0,-1) );
    send_event.emit( nv );
  } else if (m_message_type == MessageEvent::URL) {
    URLMessageEvent *uv = new URLMessageEvent( m_contact, m_url_text.get_chars(0,-1), m_url_entry.get_text() );
    send_event.emit( uv );
  } else if (m_message_type == MessageEvent::SMS) {
    SMSMessageEvent *sv = new SMSMessageEvent( m_contact, m_sms_text.get_chars(0,-1), true );
    send_event.emit( sv );
  }

}

string MessageBox::format_time(time_t t) {
  struct tm *tm = localtime(&t);
  char time_str[256];
  strftime(time_str, 255, "%H:%M:%S", tm);
  return string(time_str);
}

gint MessageBox::delete_event_impl(GdkEventAny *ev) {
  return false;
}

void MessageBox::set_status( const string& text )
{
  if( m_status.messages().size() )
    m_status.pop( m_status_context );
  m_status.push( m_status_context, text);
}
