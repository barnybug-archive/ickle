/* $Id: ContactListView.h,v 1.16 2002-01-07 21:13:52 barnabygray Exp $
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

using std::string;

using SigC::Signal1;

using namespace ICQ2000;

class ContactListView : public Gtk::CList {
 private:
  typedef Gtk::CList_Helpers::RowIterator citerator;

  struct RowData {
    unsigned int uin;
    Status status;
    unsigned int msgs;
    string alias;
  };

  Gtk::Menu rc_popup;

  void UpdateRow(const Contact& c);

  citerator lookupUIN(unsigned int uin);

  void userinfo_cb();
  void remove_user_cb();
  void fetch_away_msg_cb();
  unsigned int current_selection_uin();

 protected:
  virtual gint key_press_event_impl(GdkEventKey* ev);

 private:
  bool m_single_click, m_check_away_click;

 public:
  ContactListView();
  ~ContactListView();

  void setupAccelerators();

  void clear();

  void setSingleClick(bool b);
  void setCheckAwayClick(bool b);

  gint button_press_cb(GdkEventButton *ev);

  bool message_cb(MessageEvent *ev);
  void contactlist_cb(ContactListEvent *ev);
  void icons_changed_cb();
  void settings_changed_cb(const string&);

  static gint sort_func( GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);

  // signals
  Signal1<void,unsigned int> user_popup;
  Signal1<void,unsigned int> userinfo;
};

#endif
