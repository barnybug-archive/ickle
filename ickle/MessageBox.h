/* $Id: MessageBox.h,v 1.26 2003-01-04 19:42:46 barnabygray Exp $
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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <gtkmm/window.h>
#include <gtkmm/table.h>
#include <gtkmm/textview.h>
#include <gtkmm/paned.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/notebook.h>
#include <gtkmm/entry.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/scale.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scrolledwindow.h>

#include <string>
#include <time.h>

#include <sigc++/signal.h>

#include <libicq2000/Contact.h>
#include <libicq2000/events.h>

#include "Icons.h"
#include "History.h"
#include "MessageQueue.h"

class MessageBox : public Gtk::Window,
                   public sigslot::has_slots<>
{
 private:
  ICQ2000::ContactRef m_self_contact, m_contact;

  History *m_history;

  Gtk::Adjustment m_scaleadj;
  Gtk::HScale m_scale;
  Gtk::Label m_scalelabel;

  Gtk::VBox m_vbox_top;
  Gtk::Button m_send_button, m_close_button;

  Gtk::VBox m_history_vbox;
  Gtk::ScrolledWindow m_history_scr_win;
  Gtk::TextView m_history_text;
  guchar m_nr_shown;

  Gtk::Notebook m_tab;

  bool m_focus, m_pending;

  // normal message tab
  Gtk::TextView m_message_text;

  // url tab
  Gtk::TextView m_url_text;
  Gtk::Entry m_url_entry;

  // sms tab
  Gtk::TextView m_sms_text;
  Gtk::Entry m_sms_count;
  Gtk::Label m_sms_count_label;
  bool m_sms_count_over;
  bool m_sms_enabled, m_online;
  bool m_display_times;
  
  Gtk::RadioButton m_send_normal, m_send_urgent, m_send_tocontactlist;

  Gtk::VPaned m_pane;
  Gtk::ToggleButton m_userinfo_button, m_delivery_button;
  Gtk::HBox m_delivery_buttons;

  Gtk::Tooltips m_tooltips;
  Gtk::Statusbar m_status;
  guint m_status_context;

  ICQ2000::MessageEvent *m_last_ev;
  ICQ2000::MessageEvent::MessageType m_message_type;

  MessageQueue& m_message_queue;

  Glib::RefPtr<Gtk::TextTag> m_tag_header_blue;
  Glib::RefPtr<Gtk::TextTag> m_tag_header_red;
  Glib::RefPtr<Gtk::TextTag> m_tag_normal;

  void send_button_update();
  void set_contact_title();
  void display_message(History::Entry &he);
  void set_status( const std::string& text );
  void redraw_history();
  guint update_scalelabel(guint i);
  void scaleadj_value_changed_cb();
  bool text_button_press_cb(GdkEventButton *b, Gtk::TextView *t);
  void clear_queue();
  bool clear_queue_idle_cb();
  
  bool on_focus_in_event(GdkEventFocus* ev);
  bool on_focus_out_event(GdkEventFocus* ev);

  void history_page_up();
  void history_page_down();

  static bool is_blank(const Glib::ustring& s);

  // signals
  SigC::Signal1<void,ICQ2000::MessageEvent *> m_signal_send_event;
  SigC::Signal1<void,bool> m_signal_userinfo_dialog;
  SigC::Signal0<void> m_signal_destroy;

 public:
  MessageBox(MessageQueue& mq, const ICQ2000::ContactRef& self, const ICQ2000::ContactRef& c, History *h);
  ~MessageBox();

  void popup();

  void new_entry_cb(History::Entry *ev);

  // -- libICQ2000 callbacks   --
  void messageack_cb(ICQ2000::MessageEvent *ev);
  void contactlist_cb(ICQ2000::ContactListEvent *ev);
  void status_change_cb(ICQ2000::StatusChangeEvent *ev);

  // -- MessageQueue callbacks --
  void queue_added_cb(MessageEvent *ev);
  void queue_removed_cb(MessageEvent *ev);

  void enable_sms();
  void disable_sms();

  void online();
  void offline();

  void userinfo_dialog_cb(bool b);

  void setDisplayTimes(bool d);
  
  void raise();

  void spell_attach();
  void spell_detach();

  // signal accessors
  SigC::Signal0<void>& signal_destroy();
  SigC::Signal1<void,ICQ2000::MessageEvent *>& signal_send_event();
  SigC::Signal1<void,bool>& signal_userinfo_dialog();

  virtual void on_size_allocate(GtkAllocation*);
  virtual bool on_delete_event(GdkEventAny *ev);

  void pane_position_changed_cb(GtkAllocation*);
  void send_clicked_cb();
  void close_clicked_cb();
  void change_current_page_cb(int n);
  void userinfo_toggle_cb();
  void delivery_toggle_cb();
  void sms_count_update_cb();
  void icons_changed_cb();
  void settings_changed_cb(const std::string &key);
  bool key_press_cb(GdkEventKey*);
};

#endif
