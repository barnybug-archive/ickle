/* $Id: ContactListView.h,v 1.19 2002-03-28 18:29:02 barnabygray Exp $
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

#ifndef CONTACTLISTVIEW_H
#define CONTACTLISTVIEW_H

#include <gtk--/clist.h>
#include <gtk--/menu.h>
#include <gtk--/pixmap.h>

#include <sigc++/signal_system.h>
#include <string>

#include "main.h"
#include <libicq2000/ContactList.h>
#include <libicq2000/Contact.h>

#include "Icons.h"
#include "MessageQueue.h"

using SigC::Signal1;

class ContactListView : public Gtk::CList {
 private:
  typedef Gtk::CList_Helpers::RowIterator citerator;

  struct RowData {
    unsigned int uin;
    ICQ2000::Status status;
    unsigned int msgs;
    string alias;
  };

  Gtk::Menu rc_popup;

  MessageQueue& m_message_queue;

  void update_row(const ICQ2000::ContactRef& c);

  citerator lookupUIN(unsigned int uin);

  void userinfo_cb();
  void remove_user_cb();
  void fetch_away_msg_cb();
  unsigned int current_selection_uin();

 protected:
  virtual gint key_press_event_impl(GdkEventKey* ev);
  virtual void click_column_impl(gint);

 private:
  bool m_single_click, m_check_away_click;
  int m_sort;

 public:
  ContactListView(MessageQueue& mq);
  ~ContactListView();

  void setupAccelerators();

  void clear();

  void setSingleClick(bool b);
  void setCheckAwayClick(bool b);

  gint button_press_cb(GdkEventButton *ev);

  // -- library callbacks      --
  void contactlist_cb(ICQ2000::ContactListEvent *ev);
  void contact_userinfo_change_cb(ICQ2000::UserInfoChangeEvent *ev);
  void contact_status_change_cb(ICQ2000::StatusChangeEvent *ev);

  // -- MessageQueue callbacks --
  void queue_added_cb(MessageEvent *ev);
  void queue_removed_cb(MessageEvent *ev);
  
  // -- gui callbacks          --
  void icons_changed_cb();
  void settings_changed_cb(const std::string&);

  void load_sort_column ();
  static gint sort_func( GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
  static int status_order(ICQ2000::Status);

  // signals
  Signal1<void,unsigned int> user_popup;
  Signal1<void,unsigned int> userinfo;
};

#endif
