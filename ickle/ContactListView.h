/*
 * ContactListView
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

#include <hash_map>
#include <vector>
#include <string>
#include <utility>
#include <sstream>

#include "main.h"
#include "Client.h"
#include "ContactList.h"
#include "Contact.h"
#include "Icons.h"

using namespace Gtk;
using namespace ICQ2000;
using namespace SigC;
using namespace std;

class ContactListView : public Gtk::CList {
 private:
  typedef CList_Helpers::RowIterator citerator;

  struct RowData {
    unsigned int uin;
    Status status;
  };

  /* The CList in Gtk and RowIterator's have no guarantee they
   * remain valid between reorderings and insertions/deletions
   * so I've given up using a hash of UIN to RowIterator and
   * use a linear lookup each time instead.
   */
  //  hash_map<unsigned int,citerator> m_row_map;

  Menu rc_popup;

  void UpdateRow(const Contact& c);

  citerator lookupUIN(unsigned int uin);

 public:
  ContactListView();
  ~ContactListView();

  void clear();

  void user_info_cb();
  void remove_user_cb();
  gint button_press_cb(GdkEventButton *ev);

  bool message_cb(MessageEvent *ev);
  void contactlist_cb(ContactListEvent *ev);

  static gint sort_func( GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);

  // signals
  Signal1<void,unsigned int> user_popup;
  Signal1<void,unsigned int> user_info;
};

#endif




