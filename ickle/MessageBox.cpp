/* $Id: MessageBox.cpp,v 1.46 2002-02-26 13:48:47 barnabygray Exp $
 * 
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

#include <stdlib.h>
#include <ctype.h>

#include <algorithm>

#include "MessageBox.h"
#include "sstream_fix.h"
#include "main.h"
#include "Settings.h"

#include <gtk--/imageloader.h>
#include <gtk--/pixmap.h>
#include <gtk--/scrollbar.h>
#include <gtk--/toolbar.h>
#include <gtk--/main.h>
#include <gdk/gdkkeysyms.h>

#include <libicq2000/Client.h>

#include "gtkspell.h"

#include "PromptDialog.h"

using Gtk::Text;
using Gtk::Text_Helpers::Context;
using SigC::bind;
using SigC::slot;
using std::ostringstream;
using std::endl;
using std::exception;
using std::find_if;

MessageBox::MessageBox(Contact *self, Contact *c, History *h)
  : m_self_contact(self),
    m_contact(c),
    m_history(h),
    m_send_button("Send"), m_close_button("Close"),
    m_vbox_top(false,10),
    m_history_table(4,1,false),
    m_sms_count_label("", 0),
    m_sms_count_over(false),
    m_sms_enabled(true),
    m_scaleadj(0, 0, 0),
    m_scale(m_scaleadj),
    m_focus(false),
    m_pending(false)
{
  Gtk::Box *hbox;

  set_border_width(10);

  m_pane.set_handle_size (8);
  m_pane.set_gutter_size (12);                       
  
  // -- top pane --

  Gtk::Scrollbar *scrollbar;
  
  m_history_table.set_usize(400,140);
  m_history_table.attach(m_history_text, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,
			 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
  m_history_text.key_press_event.connect(slot(this,&MessageBox::key_press_cb));

  // scrollbars
  scrollbar = manage( new Gtk::VScrollbar (*(m_history_text.get_vadjustment())) );
  m_history_table.attach (*scrollbar, 1, 2, 0, 1, 0, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);

  // initial scale adjustment
  m_scaleadj.set_lower(0);

  m_nr_shown = g_settings.getValueUnsignedChar("history_shownr");
  m_scaleadj.set_upper( m_history->size() );

  m_scaleadj.set_step_increment(1);
  m_scaleadj.set_page_increment(m_nr_shown);
  m_scaleadj.set_page_size(m_nr_shown);
  gfloat upper =  m_scaleadj.get_upper() - m_nr_shown;
  if (upper < 0) upper = 0;
  m_scaleadj.set_value( upper );
  m_scaleadj.value_changed.connect( slot(this, &MessageBox::scaleadj_value_changed_cb) );

  // scale
  m_scale.set_draw_value( false );
  m_scale.set_update_policy(GTK_UPDATE_DELAYED);
  m_scale.set_digits(0);
  m_history_table.attach( m_scalelabel, 0, 2, 2, 3, GTK_FILL | GTK_EXPAND, GTK_FILL );
  m_history_table.attach( m_scale, 0, 2, 3, 4, GTK_FILL | GTK_EXPAND, 0 );

  m_history_text.set_editable(false);
  m_history_text.set_word_wrap(true);

  m_pane.pack1(m_history_table, true, true);

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
  if (!c->isSMSable()) disable_sms();
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

  m_pane.pack2(m_tab, false, true);

  m_vbox_top.pack_start(m_pane,true,true);

  // -- button bar --

  m_send_button.clicked.connect(slot(this,&MessageBox::send_clicked_cb));
  m_close_button.clicked.connect( destroy.slot() );
  m_tooltips.set_tip(m_send_button, "Send Message\nShortcuts: Ctrl-Enter or Alt-S");
  m_tooltips.set_tip(m_close_button, "Close window\nShortcuts: Alt-C or Escape");

  hbox = manage( new Gtk::HBox() );

  Gtk::Toolbar *toolbar = manage( new Gtk::Toolbar() );
  {
    using namespace Gtk::Toolbar_Helpers;
    ToolList& tl = toolbar->tools();
    tl.push_back( ToggleElem("User Info", slot( this, &MessageBox::userinfo_toggle_cb ), "Popup User Information Dialog" ) );
    m_userinfo_toggle = static_cast<Gtk::ToggleButton*>(tl.back()->get_widget());
  }

  hbox->pack_start(*toolbar, false);

  m_status_context = m_status.get_context_id("messagebox");
  hbox->pack_start(m_status, true, true, 5);

  m_vbox_top.pack_start(*hbox, false);

  hbox = manage( new Gtk::HButtonBox() );
  hbox->pack_start(m_send_button);
  hbox->pack_end(m_close_button);
  m_vbox_top.pack_start(*hbox,false);

  // hook up for mousewheel support
  m_history_text.button_press_event.connect( bind( slot(this, &MessageBox::text_button_press_cb), &m_history_text ) );
  m_message_text.button_press_event.connect( bind( slot(this, &MessageBox::text_button_press_cb), &m_message_text ) );
  m_url_text.button_press_event.connect( bind( slot(this, &MessageBox::text_button_press_cb), &m_url_text ) );
  m_sms_text.button_press_event.connect( bind( slot(this, &MessageBox::text_button_press_cb), &m_sms_text ) );
  
  m_histconn = m_history->new_entry.connect( slot(this, &MessageBox::new_entry_cb) );
  m_settingsconn = g_settings.settings_changed.connect( slot(this, &MessageBox::settings_changed_cb) );

  g_icons.icons_changed.connect( slot(this, &MessageBox::icons_changed_cb) );
  
  add(m_vbox_top);

  /* restore message box size */
  int message_box_width = g_settings.getValueInt("message_box_width");
  int message_box_height = g_settings.getValueInt("message_box_height");
  int message_box_pane_position = g_settings.getValueInt("message_box_pane_position");

  if ( (message_box_width > 0) && (message_box_width > 0) ) {
    set_default_size( message_box_width, message_box_height );
  } 
  if ( message_box_pane_position > 0 ) {
    m_pane.set_position( message_box_pane_position );
  }
  size_allocate.connect( slot(this, &MessageBox::resized_cb) );
  /* m_history_table height == pane position */
  m_history_table.size_allocate.connect( slot(this, &MessageBox::pane_position_changed_cb) );

  Gtk::Main::idle.connect( slot( this, &MessageBox::clear_queue_idle_cb ) );
  /* erk.. this is a kludge if ever I saw one, basically it's not safe
     after popping up a new dialog to clear out the message queue as
     it might be popped up inside callback for a message */

  realize();
  set_contact_title();
}

MessageBox::~MessageBox() {
  m_histconn.disconnect();
  m_settingsconn.disconnect();
}

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
  ostr << m_contact->getAlias();
  if (m_contact->isICQContact()) {
    ostr << " (" << m_contact->getUIN() << ")";
  }
  ostr << " - ";
  ostr << m_contact->getStatusStr();
  Gtk::ImageLoader *p;
  
  if (m_contact->numberPendingMessages() > 0) {
    ostr << "*";
    p = g_icons.IconForEvent( m_contact->getPendingMessage()->getType() );
  } else {
    p = g_icons.IconForStatus( m_contact->getStatus(), m_contact->isInvisible() );
  }
  
  gdk_window_set_icon(get_window(), NULL, p->pix(), p->bit());
  set_title(ostr.str());
}

void MessageBox::contactlist_cb(ContactListEvent *ev) {
  if (m_contact->isSMSable()) enable_sms();
  else disable_sms();

  // in case Alias or Status has changed
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

void MessageBox::userinfo_dialog_cb(bool b) {
  m_userinfo_toggle->set_active(b);
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

void MessageBox::settings_changed_cb(const string &key) {
  if( key == "history_shownr" ) {
    m_nr_shown = g_settings.getValueUnsignedChar("history_shownr");
    m_scaleadj.set_upper( m_history->size() );

    m_scaleadj.set_page_increment(m_nr_shown);
    m_scaleadj.set_page_size(m_nr_shown);
    redraw_history();
  }
}

void MessageBox::icons_changed_cb() {
  using namespace Gtk::Notebook_Helpers;
  PageList& pl = m_tab.pages();
  Gtk::ImageLoader *l;
  Gtk::Pixmap *i;
  if (m_contact->isICQContact()) {
    l = g_icons.IconForEvent(MessageEvent::Normal);
    i = manage( new Gtk::Pixmap( l->pix(), l->bit() ) );
    pl[0]->set_tab( i );
    l = g_icons.IconForEvent(MessageEvent::URL);
    i = manage( new Gtk::Pixmap( l->pix(), l->bit() ) );
    pl[1]->set_tab( i );
    l = g_icons.IconForEvent(MessageEvent::SMS);
    i = manage( new Gtk::Pixmap( l->pix(), l->bit() ) );
    pl[2]->set_tab( i );
  } else {
    l = g_icons.IconForEvent(MessageEvent::SMS);
    i = manage( new Gtk::Pixmap( l->pix(), l->bit() ) );
    pl[0]->set_tab( i );
  }
}

void MessageBox::userinfo_toggle_cb() {
  userinfo_dialog.emit( m_userinfo_toggle->get_active() );
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

void MessageBox::new_entry_cb(History::Entry *ev) {
  gfloat old_upper = m_scaleadj.get_upper() - m_nr_shown;
  if (old_upper < 0) old_upper = 0;

  m_scaleadj.set_upper( m_history->size() );

  // automatically scroll to end if they were viewing the last page
  // otherwise they were probably looking through past history, so we
  // don't want to disturb them
  if (m_scaleadj.get_value() >= old_upper) {
    gfloat upper = m_scaleadj.get_upper() - m_nr_shown;
    if (upper < 0) upper = 0;
    if (old_upper != upper) m_scaleadj.set_value( upper );
    else redraw_history();
  } else {
    update_scalelabel(m_scaleadj.get_value());
  }

  if (m_focus) {
    // already focused, empty the queue
    Gtk::Main::idle.connect( slot( this, &MessageBox::clear_queue_idle_cb ) );
  } else {
    m_pending = true;
  }
  
  set_contact_title();
}

void MessageBox::messageack_cb(MessageEvent *ev) {
  Contact *c = ev->getContact();
  if (c->getUIN() != m_contact->getUIN()) return;

  if (ev->getType() == MessageEvent::AwayMessage) return;

  if (ev->isFinished()) {
    if (ev->isDelivered()) {
      
      if (m_message_type == MessageEvent::Normal) {
	m_message_text.delete_text(0,-1);
      } else if (m_message_type == MessageEvent::URL) {
	m_url_entry.delete_text(0,-1);
	m_url_text.delete_text(0,-1);
      } else if (m_message_type == MessageEvent::SMS) {
	m_sms_text.delete_text(0,-1);
      }
      
      set_status("Sent message successfully");

      if( g_settings.getValueBool( "message_autoclose" ) )
        destroy.emit();
    } else {
      set_status("Sending message failed");
    }
  } else {
    if (ev->isDirect()) {
      set_status("Sending direct failed, sending through server");
    } else {
      set_status("Server acknowledged message");
    }
    
  }


}

void MessageBox::display_message(History::Entry &e)
{
  Context header_context, normal_context;
  
  string message_text_font = g_settings.getValueString("message_text_font");
  string message_header_font = g_settings.getValueString("message_header_font");
  if ( !message_text_font.empty() ) {
    normal_context.set_font( Gdk_Font( message_text_font ) );
  }
  if ( !message_header_font.empty() ) {
    header_context.set_font( Gdk_Font( message_header_font ) );
  }

  string nick;
  
  if( e.dir == History::Entry::SENT ) {
    header_context.set_foreground( Gdk_Color("blue") );
    nick = m_self_contact->getAlias();
  }
  else {
    header_context.set_foreground( Gdk_Color("red") );
    nick = m_contact->getAlias();
  }

  Gdk_Color white("white");
  Gdk_Color black("black");
  
  if (m_history_text.get_point() > 0)
    m_history_text.insert( normal_context, "\n");

  ostringstream ostr;
  if (m_display_times)
    ostr << format_time(e.timestamp) << " ";

  ostr << nick << " ";
  if (e.type == MessageEvent::Normal) {

    if ( e.multiparty ) ostr << "[multiparty] ";
    m_history_text.insert( header_context, ostr.str());
    m_history_text.insert( normal_context, e.message);
      
  } else if (e.type == MessageEvent::URL) {

    m_history_text.insert( header_context, ostr.str());
    m_history_text.insert( normal_context, e.URL);
    m_history_text.insert( normal_context, "\n");
    m_history_text.insert( normal_context, e.message);
      
  } else if (e.type == MessageEvent::SMS) {

    ostr << "[sms] ";
    m_history_text.insert( header_context, ostr.str());
    m_history_text.insert( normal_context, e.message);
      
  } else if (e.type == MessageEvent::SMS_Receipt) {
    if (e.delivered) {
      ostr << "[sms delivered]";
    } else {
      ostr << "[sms not delivered]";
    }
    ostr << endl;
    m_history_text.insert( header_context, ostr.str());
  }

}

void MessageBox::popup() {
  show_all();
  redraw_history();
}

struct isnotspace
{
  bool operator()(int n) { return !isspace(n); }
};

bool MessageBox::isBlank(const string& s)
{
  return (find_if(s.begin(), s.end(), isnotspace() ) == s.end());
}

void MessageBox::send_clicked_cb() {

  if (m_message_type == MessageEvent::Normal) {
    if (isBlank(m_message_text.get_chars(0,-1))) {
      PromptDialog pd(PromptDialog::PROMPT_CONFIRM, "You are about to send a blank message.\nAre you sure you wish to send it?");
      if (!pd.run()) return;
    }
    
    set_status("Sending message...");
    NormalMessageEvent *nv = new NormalMessageEvent( m_contact, m_message_text.get_chars(0,-1) );
    send_event.emit( nv );
  } else if (m_message_type == MessageEvent::URL) {
    if (isBlank(m_url_entry.get_text())) {
      PromptDialog pd(PromptDialog::PROMPT_CONFIRM, "You are about to send a blank message.\nAre you sure you wish to send it?");
      if (!pd.run()) return;
    }

    set_status("Sending URL...");
    URLMessageEvent *uv = new URLMessageEvent( m_contact, m_url_text.get_chars(0,-1), m_url_entry.get_text() );
    send_event.emit( uv );
  } else if (m_message_type == MessageEvent::SMS) {
    if (isBlank(m_sms_text.get_chars(0,-1))) {
      PromptDialog pd(PromptDialog::PROMPT_CONFIRM, "You are about to send a blank message.\nAre you sure you wish to send it?");
      if (!pd.run()) return;
    }

    set_status("Sending SMS...");
    SMSMessageEvent *sv = new SMSMessageEvent( m_contact, m_sms_text.get_chars(0,-1), true );
    send_event.emit( sv );
  }

}

string MessageBox::format_time(time_t t) {
  time_t now = time(NULL);
  struct tm now_tm = * (localtime(&now));
  struct tm tm = * (localtime(&t));
  char time_str[256];
  if (now - t > 86400 || now_tm.tm_mday != tm.tm_mday) {
    strftime(time_str, 255, "%d %b %Y %H:%M:%S", &tm);
  } else {
    strftime(time_str, 255, "%H:%M:%S", &tm);
  }
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

void MessageBox::redraw_history()
{
  scaleadj_value_changed_cb();
}

void MessageBox::scaleadj_value_changed_cb()
{
  History::Entry he;
  guint i, end;
  Gtk::Adjustment *adj;

  try {
    m_history->stream_lock();
  }
  catch ( exception &e ) {
    g_warning( e.what() );
    return;
  }

  m_history_text.freeze();
  m_history_text.delete_text(0,-1);
  i = (guint)m_scaleadj.get_value();
  end = update_scalelabel(i);
  for( ; i < end; ++i ) {
    m_history->get_msg( i, he );
    display_message( he );
  }

  m_history->stream_release();
  m_history_text.thaw();
  
  adj = m_history_text.get_vadjustment();
  adj->set_value( adj->get_upper() );
}

guint MessageBox::update_scalelabel(guint i)
{
  ostringstream os;
  guint end;
  
  end = i + m_nr_shown;
  if (end > m_history->size()) end = m_history->size();
  if ( m_history->size() == 0) {
    os << "No messages in history";
  } else {
    if (i+1 == end) os << "Message " << end;
    else os << "Messages " << i + 1 << " to " << end;
    os << " (" << m_history->size() << " total)";
  }
  m_scalelabel.set( os.str() );
  return end;
}

// provides mousewheel support for Gtk::Text controls
gint MessageBox::text_button_press_cb(GdkEventButton *b, Text *t) {
  Gtk::Adjustment *adj;
  gfloat val;

  if( b->button != 4 && b->button != 5 )
    return FALSE;
  
  adj = t->get_vadjustment();
  val = adj->get_value();
  if( b->button == 4 ) {
    val -= adj->get_page_increment();
  }
  else if( b->button = 5 ) {
    val += adj->get_page_increment();
  }
  adj->set_value( val );
  return TRUE;
}

void MessageBox::spell_attach()
{
  gtkspell_attach(GTK_TEXT(m_message_text.gtkobj()));
  gtkspell_attach(GTK_TEXT(m_url_text.gtkobj()));
  gtkspell_attach(GTK_TEXT(m_sms_text.gtkobj()));
}

void MessageBox::spell_detach()
{
  gtkspell_detach(GTK_TEXT(m_message_text.gtkobj()));
  gtkspell_detach(GTK_TEXT(m_url_text.gtkobj()));
  gtkspell_detach(GTK_TEXT(m_sms_text.gtkobj()));
}

void MessageBox::resized_cb(GtkAllocation *allocation) {
  g_settings.setValue("message_box_width", width());
  g_settings.setValue("message_box_height", height());
}

void MessageBox::pane_position_changed_cb(GtkAllocation *allocation) {
  /* Pane position is the same as m_history_table.height() */
  g_settings.setValue("message_box_pane_position", m_history_table.height());
}

gint MessageBox::focus_in_event_impl(GdkEventFocus* p0)
{
  m_focus = true;
  if (m_pending) clear_queue();
  return 0;
}

gint MessageBox::focus_out_event_impl(GdkEventFocus* p0)
{
  m_focus = false;
  return 0;
}

gint MessageBox::clear_queue_idle_cb()
{
  clear_queue();
  return 0;
}

void MessageBox::clear_queue()
{
  // clear message queue
  while (m_contact->numberPendingMessages() > 0) {
    MessageEvent *ev = m_contact->getPendingMessage();
    m_contact->erasePendingMessage(ev);
  }
  icqclient.SignalMessageQueueChanged(m_contact);

  set_contact_title();
  m_pending = false;
}
