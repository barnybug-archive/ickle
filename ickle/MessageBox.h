/* $Id: MessageBox.h,v 1.24 2002-06-16 00:01:14 barnabygray Exp $
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

#include <gtk--/window.h>
#include <gtk--/table.h>
#include <gtk--/text.h>
#include <gtk--/paned.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/buttonbox.h>
#include <gtk--/notebook.h>
#include <gtk--/entry.h>
#include <gtk--/statusbar.h>
#include <gtk--/togglebutton.h>
#include <gtk--/adjustment.h>
#include <gtk--/scale.h>
#include <gtk--/tooltips.h>
#include <gtk--/radiobutton.h>

#include <string>
#include <time.h>

#include <sigc++/signal_system.h>

#include <libicq2000/Contact.h>
#include <libicq2000/events.h>

#include "Icons.h"
#include "History.h"
#include "MessageQueue.h"

class MessageBox : public Gtk::Window {
 private:
  ICQ2000::ContactRef m_self_contact, m_contact;

  History *m_history;

  Gtk::Adjustment m_scaleadj;
  Gtk::HScale m_scale;
  Gtk::Label m_scalelabel;

  Gtk::VBox m_vbox_top;
  Gtk::Button m_send_button, m_close_button;

  Gtk::VBox m_history_vbox;
  Gtk::Text m_history_text;
  guchar m_nr_shown;

  Gtk::Notebook m_tab;

  bool m_focus, m_pending;

  // normal message tab
  Gtk::Text m_message_text;

  // url tab
  Gtk::Text m_url_text;
  Gtk::Entry m_url_entry;

  // sms tab
  Gtk::Text m_sms_text;
  Gtk::Entry m_sms_count;
  Gtk::Label m_sms_count_label;
  bool m_sms_count_over;
  bool m_sms_enabled, m_online;
  bool m_display_times;
  
  Gtk::RadioButton m_send_normal, m_send_urgent, m_send_tocontactlist;

  Gtk::VPaned m_pane;
  Gtk::ToggleButton *m_userinfo_toggle, *m_delivery_toggle;
  Gtk::HBox m_delivery_buttons;

  Gtk::Tooltips m_tooltips;
  Gtk::Statusbar m_status;
  guint m_status_context;

  ICQ2000::MessageEvent *m_last_ev;
  ICQ2000::MessageEvent::MessageType m_message_type;

  MessageQueue& m_message_queue;

  void send_button_update();
  void set_contact_title();
  std::string format_time(time_t t);
  void display_message(History::Entry &he);
  void set_status( const std::string& text );
  void redraw_history();
  guint update_scalelabel(guint i);
  void scaleadj_value_changed_cb();
  gint text_button_press_cb(GdkEventButton *b, Gtk::Text *t);
  void clear_queue();
  gint clear_queue_idle_cb();
  
  gint focus_in_event_impl(GdkEventFocus* p0);
  gint focus_out_event_impl(GdkEventFocus* p0);

  void history_page_up();
  void history_page_down();

  static bool isBlank(const std::string& s);

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
  
  void raise() const;

  void spell_attach();
  void spell_detach();

  // signals
  SigC::Signal1<void,ICQ2000::MessageEvent *> send_event;
  SigC::Signal1<void,bool> userinfo_dialog;

  void resized_cb(GtkAllocation*);
  void pane_position_changed_cb(GtkAllocation*);
  void send_clicked_cb();
  void switch_page_cb(Gtk::Notebook_Helpers::Page* p, guint n);
  void userinfo_toggle_cb();
  void delivery_toggle_cb();
  void sms_count_update_cb();
  void icons_changed_cb();
  void settings_changed_cb(const std::string &key);
  gint key_press_cb(GdkEventKey*);
  virtual gint delete_event_impl(GdkEventAny *ev);
};

#endif
