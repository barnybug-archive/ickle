/* $Id: MessageBox.cpp,v 1.75 2003-01-22 21:48:45 barnabygray Exp $
 * 
 * Copyright (C) 2001, 2002 Barnaby Gray <barnaby@beedesign.co.uk>.
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
#include "main.h"
#include "Settings.h"

#include <gtkmm/toolbar.h>
#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include <gtkmm/main.h>
#include <gtkmm/separator.h>
#include <gdk/gdkkeysyms.h>

#include <libicq2000/Client.h>

// TODO #include "gtkspell.h"

#include "PromptDialog.h"

#include "ickle.h"
#include "ucompose.h"
#include "utils.h"

#include "UserInfoHelpers.h"

#include "pixmaps/info.xpm"
#include "pixmaps/delivery.xpm"

using Gtk::TextView;

using std::string;
using std::endl;
using std::exception;
using std::find_if;

using ICQ2000::ContactRef;

MessageBox::MessageBox(MessageQueue& mq, const ICQ2000::ContactRef& self, const ICQ2000::ContactRef& c, History *h)
  : m_self_contact(self),
    m_contact(c),
    m_history(h),
    m_scaleadj(0, 0, 0),
    m_scale(m_scaleadj),
    m_vbox_top(false,10),
    m_history_vbox(false, 0),
    m_focus(false),
    m_pending(false),
    m_sms_count_label("", 0.0, 0.5),
    m_sms_count_over(false),
    m_sms_enabled(true),
    m_send_normal( _("Normal"), 0),
    m_send_urgent( _("Urgent"), 0 ),
    m_send_tocontactlist( _("To Contact List"), 0 ),
    m_last_ev(NULL),
    m_message_queue(mq)
{
  Gtk::Box *hbox;

  set_border_width(10);

  // -- top pane --
  m_history_scr_win.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
  m_history_scr_win.set_size_request(100,100); // gtk - otherwise the text widget forces it to enlarge (!?)
  m_history_scr_win.add(m_history_text);
  m_history_scr_win.set_shadow_type(Gtk::SHADOW_IN);

  // initial scale adjustment
  m_scaleadj.set_lower(0);

  m_nr_shown = g_settings.getValueUnsignedChar("history_shownr");
  m_scaleadj.set_upper( m_history->size() );

  m_scaleadj.set_step_increment(1);
  m_scaleadj.set_page_increment(m_nr_shown);
  m_scaleadj.set_page_size(m_nr_shown);
  m_scaleadj.signal_value_changed().connect( SigC::slot(*this, &MessageBox::scaleadj_value_changed_cb) );

  // scale
  m_scale.set_draw_value( false );
  m_scale.set_update_policy(Gtk::UPDATE_DELAYED);
  m_scale.set_digits(0);

  m_history_vbox.pack_start(m_history_scr_win, true, true);
  m_history_vbox.pack_start(m_scalelabel, false, false);
  m_history_vbox.pack_start(m_scale, false, false);

  m_history_text.set_editable(false);
  m_history_text.set_cursor_visible(false);
  m_history_text.set_wrap_mode(Gtk::WRAP_WORD);

  m_pane.pack1(m_history_vbox, true, false);

  // -- bottom pane --

  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  Gtk::Image *img;

  // tab index

  m_tab.set_tab_pos(Gtk::POS_LEFT);

  Gtk::ScrolledWindow *scrolled_win;

  if ( c->isICQContact() )
  {
    m_message_type = ICQ2000::MessageEvent::Normal;
    m_tab.signal_change_current_page().connect(SigC::slot(*this,&MessageBox::change_current_page_cb));

    // -- normal message tab --
    scrolled_win = manage( new Gtk::ScrolledWindow() );
    scrolled_win->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    scrolled_win->set_size_request(100, 100); // gtk - otherwise the text widget forces it to enlarge (!?)
    scrolled_win->add(m_message_text);
    scrolled_win->set_shadow_type(Gtk::SHADOW_IN);
  
    m_message_text.set_wrap_mode(Gtk::WRAP_WORD);
    m_message_text.set_editable(true);
    m_message_text.signal_key_press_event().connect(SigC::slot(*this,&MessageBox::key_press_cb), false /* connect before widget gets it */);

    pixbuf = g_icons.get_icon_for_event(ICQMessageEvent::Normal);
    img = manage( new Gtk::Image(pixbuf) );

    m_tab.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *scrolled_win, *img )  );
    // -------------------------

    // -------- url tab --------
    Gtk::Box *url_hbox = manage( new Gtk::HBox() );
    Gtk::Box *url_vbox = manage( new Gtk::VBox() );

    scrolled_win = manage( new Gtk::ScrolledWindow() );
    scrolled_win->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    scrolled_win->set_size_request(100,100); // gtk - otherwise the text widget forces it to enlarge (!?)
    scrolled_win->add(m_url_text);
    scrolled_win->set_shadow_type(Gtk::SHADOW_IN);
  
    m_url_text.set_wrap_mode(Gtk::WRAP_WORD);
    m_url_text.set_editable(true);
    m_url_text.signal_key_press_event().connect(SigC::slot(*this,&MessageBox::key_press_cb), false /* connect before widget gets it */);

    url_vbox->pack_start( *scrolled_win, Gtk::PACK_EXPAND_WIDGET );

    url_hbox->pack_start( * manage( new Gtk::Label( _("URL:") ) ), Gtk::PACK_SHRINK );
    url_hbox->pack_start( m_url_entry, Gtk::PACK_EXPAND_WIDGET );

    url_vbox->pack_start( *url_hbox, Gtk::PACK_SHRINK );

    pixbuf = g_icons.get_icon_for_event(ICQMessageEvent::URL);
    img = manage( new Gtk::Image(pixbuf) );

    m_tab.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *url_vbox, *img )  );
    // -------------------------
  } else {
    m_message_type = ICQ2000::MessageEvent::SMS;
  }

  // -------- sms tab --------
  Gtk::Box *sms_hbox = manage( new Gtk::HBox() );
  Gtk::Box *sms_vbox = manage( new Gtk::VBox() );

  scrolled_win = manage( new Gtk::ScrolledWindow() );
  scrolled_win->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
  scrolled_win->set_size_request(100,100); // gtk - otherwise the text widget forces it to enlarge (!?)
  scrolled_win->add(m_sms_text);
  scrolled_win->set_shadow_type(Gtk::SHADOW_IN);
  
  m_sms_text.set_wrap_mode(Gtk::WRAP_WORD);
  m_sms_text.set_editable(true);
  m_sms_text.get_buffer()->signal_changed().connect( SigC::slot( *this, &MessageBox::sms_count_update_cb ) );
  m_sms_text.signal_key_press_event().connect(SigC::slot(*this,&MessageBox::key_press_cb), false /* connect before widget gets it */);
  if (!c->isSMSable()) disable_sms();
  sms_count_update_cb();

  sms_vbox->pack_start( *scrolled_win, Gtk::PACK_EXPAND_WIDGET );

  m_sms_count.set_editable(false);
  m_sms_count.set_width_chars(3);
  sms_hbox->pack_start( m_sms_count, Gtk::PACK_SHRINK );

  m_sms_count_label.set_justify(Gtk::JUSTIFY_LEFT);
  sms_hbox->pack_start( m_sms_count_label, Gtk::PACK_EXPAND_WIDGET );

  sms_vbox->pack_start( *sms_hbox, Gtk::PACK_SHRINK );

  pixbuf = g_icons.get_icon_for_event(ICQMessageEvent::SMS);
  img = manage( new Gtk::Image(pixbuf) );

  m_tab.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *sms_vbox, *img )  );
  // -------------------------

  Gtk::VBox *pane_vbox = manage( new Gtk::VBox() );
  pane_vbox->pack_start(m_tab);
  
  // -- sending modes --
  
  if (m_contact->getStatus() == ICQ2000::STATUS_DND
      || m_contact->getStatus() == ICQ2000::STATUS_OCCUPIED)
    m_send_tocontactlist.set_active(true);

  Gtk::RadioButton::Group gp = m_send_normal.get_group();
  m_send_urgent.set_group(gp);
  m_send_tocontactlist.set_group(gp);
  if (m_contact->getStatus() == ICQ2000::STATUS_DND
      || m_contact->getStatus() == ICQ2000::STATUS_OCCUPIED)
  {
    m_send_tocontactlist.set_active(true);
  }

  m_delivery_buttons.pack_end( m_send_normal, Gtk::PACK_SHRINK );
  m_delivery_buttons.pack_end( m_send_urgent, Gtk::PACK_SHRINK );
  m_delivery_buttons.pack_end( m_send_tocontactlist, Gtk::PACK_SHRINK );

  pane_vbox->pack_start( m_delivery_buttons );

  m_pane.pack2(*pane_vbox, false, false);

  m_vbox_top.pack_start(m_pane, true, true);

  // -- status bar --

  m_status_context = m_status.get_context_id("messagebox");
  m_status.set_has_resize_grip(false);

  // -- buttons --

  hbox = manage( new Gtk::HBox(false, 5) );

  Glib::RefPtr<Gdk::Pixbuf> info_pixbuf = Gdk::Pixbuf::create_from_xpm_data( info_xpm );
  m_userinfo_button.add( * manage( new Gtk::Image( info_pixbuf ) ) );
  m_userinfo_button.signal_toggled().connect( SigC::slot( *this, &MessageBox::userinfo_toggle_cb ) );
  m_tooltips.set_tip(m_userinfo_button, _("Popup User Information Dialog") );

  Glib::RefPtr<Gdk::Pixbuf> delivery_pixbuf = Gdk::Pixbuf::create_from_xpm_data( delivery_xpm );
  m_delivery_button.add( * manage( new Gtk::Image( delivery_pixbuf ) ) );
  m_delivery_button.signal_toggled().connect( SigC::slot( *this, &MessageBox::delivery_toggle_cb ) );
  m_tooltips.set_tip(m_delivery_button, _("Show/Hide Delivery urgency options") );

  hbox->pack_start( m_status, Gtk::PACK_EXPAND_WIDGET );
  hbox->pack_start( m_userinfo_button, Gtk::PACK_SHRINK);
  hbox->pack_start( m_delivery_button, Gtk::PACK_SHRINK);

  Glib::RefPtr<Gdk::Pixbuf> send_pixbuf = g_icons.get_icon_for_event(ICQMessageEvent::Normal);
  m_send_button.add( * manage( new Gtk::Image( send_pixbuf ) ) );
  m_send_button.signal_clicked().connect(  SigC::slot( *this, &MessageBox::send_clicked_cb ) );
  m_tooltips.set_tip(m_send_button, _("Send Message\nShortcuts: Ctrl-Enter or Alt-S") );

  m_close_button.add( * manage( new Gtk::Image(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU) ) );
  m_close_button.signal_clicked().connect( SigC::slot( *this, &MessageBox::close_clicked_cb ) );
  m_tooltips.set_tip(m_close_button, _("Close window\nShortcuts: Alt-C or Escape") );

  hbox->pack_end( m_close_button, Gtk::PACK_SHRINK);
  hbox->pack_end( m_send_button, Gtk::PACK_SHRINK);
  //  hbox->pack_end( * manage( new Gtk::VSeparator() ), Gtk::PACK_SHRINK );

  m_vbox_top.pack_start(*hbox, Gtk::PACK_SHRINK);

  // hook up for mousewheel support
  /* TODO
  m_history_text.signal_button_press_event().connect( SigC::bind( SigC::slot(*this, &MessageBox::text_button_press_cb), &m_history_text ) );
  m_message_text.signal_button_press_event().connect( SigC::bind( SigC::slot(*this, &MessageBox::text_button_press_cb), &m_message_text ) );
  m_url_text.signal_button_press_event().connect( SigC::bind( SigC::slot(*this, &MessageBox::text_button_press_cb), &m_url_text ) );
  m_sms_text.signal_button_press_event().connect( SigC::bind( SigC::slot(*this, &MessageBox::text_button_press_cb), &m_sms_text ) );
  */
  
  m_history->new_entry.connect( SigC::slot(*this, &MessageBox::new_entry_cb) );
  g_settings.settings_changed.connect( SigC::slot(*this, &MessageBox::settings_changed_cb) );

  g_icons.icons_changed.connect( SigC::slot(*this, &MessageBox::icons_changed_cb) );
  
  add(m_vbox_top);

  /* restore message box size */
  int message_box_width = g_settings.getValueInt("message_box_width");
  int message_box_height = g_settings.getValueInt("message_box_height");
  int message_box_pane_position = g_settings.getValueInt("message_box_pane_position");

  if ( (message_box_width > 0) && (message_box_width > 0) )
  {
    set_default_size( message_box_width, message_box_height );
  } 

  if ( message_box_pane_position > 0 )
  {
    m_pane.set_position( message_box_pane_position );
  }

  /* m_history_table height == pane position */
  m_history_vbox.signal_size_allocate().connect( SigC::slot(*this, &MessageBox::pane_position_changed_cb) );

  // -- callbacks for libicq2000   --
  m_contact->status_change_signal.connect( this, &MessageBox::status_change_cb );

  // -- callbacks for MessageQueue --
  m_message_queue.added.connect( SigC::slot( *this, &MessageBox::queue_added_cb ) );
  m_message_queue.removed.connect( SigC::slot( *this, &MessageBox::queue_removed_cb ) );

  Glib::signal_idle().connect( SigC::slot( *this, &MessageBox::clear_queue_idle_cb ) );
  /* erk.. this is a kludge if ever I saw one, basically it's not safe
     after popping up a new dialog to clear out the message queue as
     it might be popped up inside callback for a message */

  /* history font tags
     important: define tags before displaying history */
  Glib::RefPtr<Gtk::TextBuffer> buffer = m_history_text.get_buffer();
  m_tag_header_blue = buffer->create_tag("heading_blue");
  m_tag_header_red  = buffer->create_tag("heading_red");
  m_tag_normal      = buffer->create_tag("normal");

  m_tag_header_blue->property_foreground().set_value( "blue" );
  m_tag_header_red->property_foreground().set_value( "red" );

  // scroll down to the end of the history
  gfloat upper =  m_scaleadj.get_upper() - m_nr_shown;
  if (upper < 0) upper = 0;
  m_scaleadj.set_value( upper );

  show_all();
  m_delivery_buttons.hide_all(); // start hidden
  set_contact_title();
}

MessageBox::~MessageBox()
{
  m_signal_destroy.emit();
}

void MessageBox::raise()
{
  get_window()->show();
}

bool MessageBox::key_press_cb(GdkEventKey* ev)
{
  if (ev->state & GDK_CONTROL_MASK)
  {
    if (ev->keyval == GDK_Page_Up)
    {
      history_page_up();
      return true;
    }
    else if (ev->keyval == GDK_Page_Down)
    {
      history_page_down();
      return true;
    }
  }

  // key shortcuts dependent on online/offline
  if (m_online)
  {
    if (ev->state & GDK_CONTROL_MASK )
    {
      if (ev->keyval == GDK_Return || ev->keyval == GDK_KP_Enter)
      {
        m_send_button.clicked();
	return true;
      }
      
    }
    else if (ev->state & GDK_MOD1_MASK)
    {
      if (ev->keyval == GDK_s)
      {
        m_send_button.clicked();
	return true;
      }
      
    }
  }

  if ( (ev->state & GDK_MOD1_MASK && ev->keyval == GDK_c )
       || ev->keyval == GDK_Escape)
    delete this;

  return false;
}

void MessageBox::history_page_up()
{
  Gtk::Adjustment *adj = m_history_scr_win.get_vadjustment();
  gfloat p = adj->get_value();
  
  if (p <= adj->get_lower())
  {
    // horizontal scroller
    p = m_scaleadj.get_value();
    p -= m_scaleadj.get_page_increment();
    if (p < m_scaleadj.get_lower()) p = m_scaleadj.get_lower();

    if (p != m_scaleadj.get_value())
    {
      m_scaleadj.set_value(p);
      adj->set_value( adj->get_upper() );
    }
    
  }
  else
  {
    // vertical scroller
    p -= adj->get_page_increment();
    if (p < adj->get_lower()) p = adj->get_lower();
    adj->set_value( p );
  }
}

void MessageBox::history_page_down()
{
  Gtk::Adjustment *adj = m_history_scr_win.get_vadjustment();
  gfloat p = adj->get_value();
  if (p + adj->get_page_size() >= adj->get_upper()) {
    // horizontal scroller
    p = m_scaleadj.get_value();
    p += m_scaleadj.get_page_increment();
    if (p + m_scaleadj.get_page_size() > m_scaleadj.get_upper())
      p = m_scaleadj.get_upper() - m_scaleadj.get_page_size();
    if (p < m_scaleadj.get_lower()) p = m_scaleadj.get_lower();

    if (p != m_scaleadj.get_value()) {
      m_scaleadj.set_value(p);
      adj->set_value( adj->get_lower() );
    }
    
  } else {
    // vertical scroller
    p += adj->get_page_increment();
    if (p + adj->get_page_size() >= adj->get_upper()) p = adj->get_upper() - adj->get_page_size();
    if (p < adj->get_lower()) p = adj->get_lower();
    adj->set_value( p );
  }
}

void MessageBox::set_contact_title()
{
  Glib::ustring str;

  if (m_contact->isICQContact())
    str = String::ucompose( "%1 (%2) - %3",
			    m_contact->getNameAlias(),
			    m_contact->getUIN(),
			    UserInfoHelpers::getStringFromStatus(m_contact->getStatus()) );
  else
    str = String::ucompose( "%1 - %2",
			    m_contact->getNameAlias(),
			    UserInfoHelpers::getStringFromStatus(m_contact->getStatus()) );

  Glib::RefPtr<Gdk::Pixbuf> p;

  if (g_settings.getValueBool("window_status_icons"))
  {
    if (m_message_queue.get_contact_size(m_contact) > 0)
    {
      str += "*";
      MessageEvent *ev = m_message_queue.get_contact_first_message(m_contact);
      if (ev->getServiceType() == MessageEvent::ICQ)
      {
        ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
        p = g_icons.get_icon_for_event(icq->getICQMessageType());
      }
    }
    else
    {
      p = g_icons.get_icon_for_status( m_contact->getStatus(), m_contact->isInvisible() );
    }

    set_icon(p);
  }

  set_title(str);
}

void MessageBox::contactlist_cb(ICQ2000::ContactListEvent *)
{
  if (m_contact->isSMSable())
    enable_sms();
  else
    disable_sms();

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

void MessageBox::userinfo_dialog_cb(bool b)
{
  m_userinfo_button.set_active(b);
}

void MessageBox::setDisplayTimes(bool d)
{
  m_display_times = d;
}

void MessageBox::send_button_update()
{
  if (m_message_type == ICQ2000::MessageEvent::SMS)
  {
    if (m_sms_enabled && m_online && !m_sms_count_over)
      m_send_button.set_sensitive(true);
    else
      m_send_button.set_sensitive(false);
  }
  else
  {
    if (m_online)
      m_send_button.set_sensitive(true);
    else
      m_send_button.set_sensitive(false);
  }
}

void MessageBox::sms_count_update_cb()
{
  guint len = m_sms_text.get_buffer()->get_char_count();
  // UTF-8 consideration? - for that matter, how does UTF-8 work with SMS ??

  Glib::ustring str;
  if (len > ICQ2000::SMS_Max_Length)
  {
    m_sms_count_over = true;
    str = String::ucompose( "%1", (len - ICQ2000::SMS_Max_Length) );
    m_sms_count_label.set_text( _("chars over") );
    send_button_update();
  }
  else
  {
    m_sms_count_over = false;
    str = String::ucompose( "%1", (ICQ2000::SMS_Max_Length - len) );
    m_sms_count_label.set_text( _("chars left") );
    send_button_update();
  }
  m_sms_count.set_text( str );
}

void MessageBox::settings_changed_cb(const string &key)
{
  if( key == "history_shownr" )
  {
    m_nr_shown = g_settings.getValueUnsignedChar("history_shownr");
    m_scaleadj.set_upper( m_history->size() );

    m_scaleadj.set_page_increment(m_nr_shown);
    m_scaleadj.set_page_size(m_nr_shown);
    redraw_history();
  }
}

void MessageBox::icons_changed_cb()
{
  using namespace Gtk::Notebook_Helpers;
  PageList& pl = m_tab.pages();

  Glib::RefPtr<Gdk::Pixbuf> l;
  Gtk::Image *i;
  if (m_contact->isICQContact())
  {
    l = g_icons.get_icon_for_event(ICQMessageEvent::Normal);
    i = manage( new Gtk::Image(l) );
    pl[0].set_tab_label( *i ); // MM ?
    l = g_icons.get_icon_for_event(ICQMessageEvent::URL);
    i = manage( new Gtk::Image(l) );
    pl[1].set_tab_label( *i );
    l = g_icons.get_icon_for_event(ICQMessageEvent::SMS);
    i = manage( new Gtk::Image(l) );
    pl[2].set_tab_label( *i );
  }
  else
  {
    l = g_icons.get_icon_for_event(ICQMessageEvent::SMS);
    i = manage( new Gtk::Image(l) );
    pl[0].set_tab_label( *i );
  }

  // update title icon
  set_contact_title();
}

void MessageBox::userinfo_toggle_cb()
{
  m_signal_userinfo_dialog.emit( m_userinfo_button.get_active() );
}

void MessageBox::delivery_toggle_cb()
{
  if ( m_delivery_button.get_active() )
    m_delivery_buttons.show_all();
  else
    m_delivery_buttons.hide_all();
}

void MessageBox::change_current_page_cb(int n)
{
  if (n == 0 && m_contact->isICQContact() )
  {
    m_message_type = ICQ2000::MessageEvent::Normal;
    m_message_text.grab_focus();
  }
  else if ( n == 1 )
  {
    m_message_type = ICQ2000::MessageEvent::URL;
    m_url_text.grab_focus();
  }
  else if ( n == 2 || ( n == 0 && !m_contact->isICQContact() ) )
  {
    m_message_type = ICQ2000::MessageEvent::SMS;
    m_sms_text.grab_focus();
    sms_count_update_cb();
  }

  send_button_update();
}

void MessageBox::new_entry_cb(History::Entry *)
{
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
    update_scalelabel((guint)m_scaleadj.get_value());
  }

  if (m_focus)
  {
    // already focused, empty the queue
    Glib::signal_idle().connect( SigC::slot( *this, &MessageBox::clear_queue_idle_cb ) );
  } else {
    m_pending = true;
  }
  
  set_contact_title();
}

void MessageBox::messageack_cb(ICQ2000::MessageEvent *ev)
{
  if (ev != m_last_ev) return;
  if (ev->getType() == ICQ2000::MessageEvent::AwayMessage) return;

  if (ev->isFinished()) {
    if (ev->isDelivered()) {
      
      string method;
      if (ev->isDirect())
      {
	set_status( _("Sent message direct") );
      }
      else
      {
	ICQ2000::ICQMessageEvent *cev = dynamic_cast<ICQ2000::ICQMessageEvent*>(ev);
	if ( cev != NULL )
	{
	  if (cev->isOfflineMessage())
	    set_status( _("Sent message offline") );
	  else
	    set_status( _("Sent message through server") );
	}
      }

      if( g_settings.getValueBool( "message_autoclose" ) )
	delete this;

    } else {
      switch(ev->getDeliveryFailureReason()) {
      case ICQ2000::MessageEvent::Failed_Denied:
	set_status( _("Sending message failed - user is ignoring you") );
	break;
      case ICQ2000::MessageEvent::Failed_Occupied:
	set_status( _("Sending message failed - user is occupied") );
	break;
      case ICQ2000::MessageEvent::Failed_DND:
	set_status( _("Sending message failed - user is in do not disturb") );
	break;
      case ICQ2000::MessageEvent::Failed_NotConnected:
	set_status( _("Sending message failed - you are not connected") );
	break;
      case ICQ2000::MessageEvent::Failed:
      default:
	set_status( _("Sending message failed") );
      }
    }
  } else {
    if (ev->isDirect()) {
      set_status( _("Sending direct failed, sending through server") );
    } else {
      set_status( _("Server acknowledged message") );
    }
    
  }


}

void MessageBox::display_message(History::Entry &e)
{
  Glib::RefPtr<Gtk::TextBuffer> buffer = m_history_text.get_buffer();
  Glib::RefPtr<Gtk::TextBuffer::Tag> tag_header;

  /*  
  string message_text_font = g_settings.getValueString("message_text_font");
  string message_header_font = g_settings.getValueString("message_header_font");
  if ( !message_text_font.empty() )
  {
    normal_tag->property_font().set_value( message_text_font );
  }
  if ( !message_header_font.empty() )
  {
    header_tag->property_font().set_value( message_header_font );
  }
  */

  string nick;

  if( e.dir == History::Entry::SENT )
  {
    tag_header = m_tag_header_blue;
    nick = m_self_contact->getAlias();
  }
  else
  {
    tag_header = m_tag_header_red;
    nick = m_contact->getAlias();
  }

  // new-line between messages if not the first
  if (buffer->size() > 0)
    buffer->insert_with_tag( buffer->end(), "\n", m_tag_normal );

  if (m_display_times)
  {
    buffer->insert_with_tag( buffer->end(), Utils::format_time(e.timestamp), tag_header);
    buffer->insert_with_tag( buffer->end(), " ", tag_header);
  }

  buffer->insert_with_tag( buffer->end(), nick, tag_header);
  buffer->insert_with_tag( buffer->end(), " ", tag_header);

  if (e.urgent)
  {
    buffer->insert_with_tag( buffer->end(), _("[urgent] "), tag_header);
  }

  switch(e.type)
  {

  case ICQ2000::MessageEvent::Normal:
    if ( e.multiparty )
      buffer->insert_with_tag( buffer->end(), _("[multiparty] "), tag_header);

    buffer->insert_with_tag( buffer->end(), e.message,  m_tag_normal);
    break;
    
  case ICQ2000::MessageEvent::URL:
    buffer->insert_with_tag( buffer->end(), e.URL,      m_tag_normal);
    buffer->insert_with_tag( buffer->end(), "\n",       m_tag_normal);
    buffer->insert_with_tag( buffer->end(), e.message,  m_tag_normal);
    break;
    
  case ICQ2000::MessageEvent::SMS:
    buffer->insert_with_tag( buffer->end(), _("[sms] "), tag_header);
    buffer->insert_with_tag( buffer->end(), e.message, m_tag_normal);
    break;
      
  case ICQ2000::MessageEvent::SMS_Receipt:
    if (e.delivered)
      buffer->insert_with_tag( buffer->end(), _("[sms delivered]\n"), tag_header);
    else
      buffer->insert_with_tag( buffer->end(), _("[sms not delivered] \n"), tag_header);

    break;
    
  case ICQ2000::MessageEvent::EmailEx:
    buffer->insert_with_tag( buffer->end(), _("[email] "), tag_header);
    buffer->insert_with_tag( buffer->end(), e.message, m_tag_normal);
    break;
      
  case ICQ2000::MessageEvent::WebPager:
    buffer->insert_with_tag( buffer->end(), _("[pager] "), tag_header);
    buffer->insert_with_tag( buffer->end(), e.message, m_tag_normal);
    break;
      
  default:
    break;
  }

}

void MessageBox::popup()
{
  show_all();
  if (!m_delivery_button.get_active()) m_delivery_buttons.hide_all();
  redraw_history();
}

struct isnotspace
{
  bool operator()(int n) { return !isspace(n); } // UTF-8 consideration?
};

bool MessageBox::is_blank(const Glib::ustring& s)
{
  return (find_if(s.begin(), s.end(), isnotspace() ) == s.end());
}

void MessageBox::send_clicked_cb()
{
  if (m_message_type == ICQ2000::MessageEvent::Normal)
  {
    Glib::RefPtr<Gtk::TextBuffer> buffer = m_message_text.get_buffer();

    if (is_blank(buffer->get_text()))
    {
      PromptDialog pd(*this, Gtk::MESSAGE_QUESTION, _("You are about to send a blank message.\nAre you sure you wish to send it?") );
      if (pd.run() != Gtk::RESPONSE_YES) return;
    }
    
    set_status( _("Sending message...") );
    ICQ2000::NormalMessageEvent *nv = new ICQ2000::NormalMessageEvent( m_contact, buffer->get_text() );

    if (m_send_urgent.get_active())
      nv->setUrgent(true);

    if (m_send_tocontactlist.get_active())
      nv->setToContactList(true);

    m_last_ev = nv;
    m_signal_send_event.emit( nv );
    buffer->erase( buffer->begin(), buffer->end() );
  }
  else if (m_message_type == ICQ2000::MessageEvent::URL)
  {
    Glib::RefPtr<Gtk::TextBuffer> buffer = m_url_text.get_buffer();

    if (is_blank(buffer->get_text()))
    {
      PromptDialog pd(*this, Gtk::MESSAGE_QUESTION, _("You are about to send a blank message.\nAre you sure you wish to send it?") );
      if (pd.run() != Gtk::RESPONSE_YES) return;
    }

    set_status( _("Sending URL...") );
    ICQ2000::URLMessageEvent *uv = new ICQ2000::URLMessageEvent( m_contact, buffer->get_text(), m_url_entry.get_text() );

    if (m_send_urgent.get_active())
      uv->setUrgent(true);

    if (m_send_tocontactlist.get_active())
      uv->setToContactList(true);

    m_last_ev = uv;
    m_signal_send_event.emit( uv );
    m_url_entry.delete_text( 0, -1 );
    buffer->erase( buffer->begin(), buffer->end() );
  }
  else if (m_message_type == ICQ2000::MessageEvent::SMS)
  {
    Glib::RefPtr<Gtk::TextBuffer> buffer = m_sms_text.get_buffer();

    if (is_blank(buffer->get_text()))
    {
      PromptDialog pd(*this, Gtk::MESSAGE_QUESTION, _("You are about to send a blank message.\nAre you sure you wish to send it?") );
      if (pd.run() != Gtk::RESPONSE_YES) return;
    }

    set_status( _("Sending SMS...") );
    ICQ2000::SMSMessageEvent *sv = new ICQ2000::SMSMessageEvent( m_contact, buffer->get_text(), true );
    m_last_ev = sv;
    m_signal_send_event.emit( sv );
    buffer->erase( buffer->begin(), buffer->end() );
  }

}

void MessageBox::close_clicked_cb()
{
  delete this;
}

void MessageBox::set_status( const string& text )
{
  m_status.pop( m_status_context );
  m_status.push( text, m_status_context );
}

void MessageBox::redraw_history()
{
  scaleadj_value_changed_cb();
}

void MessageBox::scaleadj_value_changed_cb()
{
  History::Entry he;
  guint i, end;

  try
  {
    m_history->stream_lock();
  }
  catch ( exception &e )
  {
    g_warning( e.what() );
    return;
  }

  Glib::RefPtr<Gtk::TextBuffer> buffer = m_history_text.get_buffer();
  buffer->erase( buffer->begin(), buffer->end() );

  i = (guint)m_scaleadj.get_value();
  end = update_scalelabel(i);

  Gtk::TextIter iter = buffer->begin();
  m_history_text.scroll_to_iter( iter, 0.0, 0.0, 0.0 );

  Glib::RefPtr<Gtk::TextBuffer::Mark> mark;

  for( ; i < end; ++i )
  {
    m_history->get_msg( i, he );

    if (i == end - 1 && end == m_scaleadj.get_upper())
    {
      // auto-scroll last message to top, if they're on the last page
      Glib::RefPtr<Gtk::TextBuffer::Mark> mark = buffer->create_mark( buffer->end(), true );
      display_message( he );
      m_history_text.scroll_to_mark( mark, 0.0, 0.0, 0.0 );
      buffer->delete_mark( mark );
    }
    else
    {
      display_message( he );
    }
  }

  m_history->stream_release();
}

guint MessageBox::update_scalelabel(guint i)
{
  Glib::ustring str;
  guint end;
  
  end = i + m_nr_shown;
  if (end > m_history->size()) end = m_history->size();
  if ( m_history->size() == 0)
  {
    str = _("No messages in history");
  }
  else
  {
    if (i+1 == end)
      str = String::ucompose( _("Message %1 (%2 total)"), end, m_history->size() );
    else
      str = String::ucompose( _("Messages %1 to %2 (%3 total)"), i + 1, end, m_history->size() );
  }
  m_scalelabel.set_text( str );
  return end;
}

// provides mousewheel support for Gtk::TextView controls
/* TODO!
bool MessageBox::text_button_press_cb(GdkEventButton *b, Text *t)
{
  Gtk::Adjustment *adj;
  gfloat val;

  if( b->button != 4 && b->button != 5 )
    return false;
  
  adj = t->get_vadjustment();
  val = adj->get_value();
  if( b->button == 4 )
  {
    val -= adj->get_page_increment();
  }
  else if( b->button == 5 )
  {
    val += adj->get_page_increment();
  }
  adj->set_value( val );

  return true;
}
*/

void MessageBox::spell_attach()
{
  /* TODO
  gtkspell_attach(Gtk::TEXT(m_message_text.gtkobj()));
  gtkspell_attach(Gtk::TEXT(m_url_text.gtkobj()));
  gtkspell_attach(Gtk::TEXT(m_sms_text.gtkobj()));
  */
}

void MessageBox::spell_detach()
{
  /* TODO
  gtkspell_detach(Gtk::TEXT(m_message_text.gtkobj()));
  gtkspell_detach(Gtk::TEXT(m_url_text.gtkobj()));
  gtkspell_detach(Gtk::TEXT(m_sms_text.gtkobj()));
  */
}

void MessageBox::on_size_allocate(GtkAllocation *al)
{
  g_settings.setValue("message_box_width", al->width);
  g_settings.setValue("message_box_height", al->height);

  Gtk::Window::on_size_allocate(al);
}

bool MessageBox::on_delete_event(GdkEventAny *ev)
{
  delete this;
  return true;
}

void MessageBox::pane_position_changed_cb(GtkAllocation *)
{
  /* Pane position is the same as m_history_table.height() */
  g_settings.setValue("message_box_pane_position", m_history_vbox.get_height());
}

bool MessageBox::on_focus_in_event(GdkEventFocus* ev)
{
  m_focus = true;
  if (m_pending)
    clear_queue();

  return Gtk::Window::on_focus_in_event(ev);
}

bool MessageBox::on_focus_out_event(GdkEventFocus* ev)
{
  m_focus = false;

  return Gtk::Window::on_focus_out_event(ev);
}

bool MessageBox::clear_queue_idle_cb()
{
  clear_queue();
  return false;
}

void MessageBox::clear_queue()
{
  MessageQueue::iterator next, curr = m_message_queue.begin();
  while (curr != m_message_queue.end()) {
    next = curr;
    ++next;

    if ((*curr)->getServiceType() == MessageEvent::ICQ) {
      ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(*curr);
      ICQMessageEvent::ICQMessageType t = icq->getICQMessageType();

      // only empty queue of event types handled by the MessageBox
      // others, such as AuthReq we shouldn't touch
      if (m_contact->getUIN() == icq->getICQContact()->getUIN()
	  && (t == ICQMessageEvent::Normal
	      || t == ICQMessageEvent::URL
	      || t == ICQMessageEvent::SMS
	      || t == ICQMessageEvent::SMS_Receipt
	      || t == ICQMessageEvent::WebPager
	      || t == ICQMessageEvent::EmailEx))
	m_message_queue.remove_from_queue(curr);
    }
    
    curr = next;
  }
  m_pending = false;
}

void MessageBox::queue_added_cb(MessageEvent *ev)
{
  if (ev->getServiceType() != MessageEvent::ICQ) return;
  ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
  if (icq->getICQContact()->getUIN() == m_contact->getUIN())
    set_contact_title();
}

void MessageBox::queue_removed_cb(MessageEvent *ev)
{
  if (ev->getServiceType() != MessageEvent::ICQ) return;
  ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
  if (icq->getICQContact()->getUIN() == m_contact->getUIN())
    set_contact_title();
}

void MessageBox::status_change_cb(ICQ2000::StatusChangeEvent *ev)
{
  // automatically change the send as over - a nice touch :-)
  if ((ev->getStatus() == ICQ2000::STATUS_DND || ev->getStatus() == ICQ2000::STATUS_OCCUPIED)
      && !m_send_urgent.get_active()) m_send_tocontactlist.set_active(true);

  set_contact_title();
}

SigC::Signal0<void>& MessageBox::signal_destroy()
{
  return m_signal_destroy;
}

SigC::Signal1<void,ICQ2000::MessageEvent *>& MessageBox::signal_send_event()
{
  return m_signal_send_event;
}

SigC::Signal1<void,bool>& MessageBox::signal_userinfo_dialog()
{
  return m_signal_userinfo_dialog;
}

