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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EXT_HASH_MAP
# include <ext/hash_map>
#elif HAVE_HASH_MAP
# include <hash_map>
#else
# error "hash_map not defined"
#endif

#include "main.h"
#include "ContactList.h"
#include "Contact.h"
#include "Icons.h"

using SigC::Signal1;

using namespace ICQ2000;

class ContactListView : public Gtk::CList {
 private:
  typedef Gtk::CList_Helpers::RowIterator citerator;

  struct RowData {
    unsigned int uin;
    Status status;
    unsigned int msgs;
  };

  /* The CList in Gtk and RowIterator's have no guarantee they
   * remain valid between reorderings and insertions/deletions
   * so I've given up using a hash of UIN to RowIterator and
   * use a linear lookup each time instead.
   */
  //  hash_map<unsigned int,citerator> m_row_map;

  Gtk::Menu rc_popup;

  void UpdateRow(const Contact& c);

  citerator lookupUIN(unsigned int uin);

  void userinfo_cb();
  void remove_user_cb();
  void fetch_away_msg_cb();
  unsigned int current_selection_uin();

 public:
  ContactListView();
  ~ContactListView();

  void setupAccelerators();

  void clear();

  gint button_press_cb(GdkEventButton *ev);

  bool message_cb(MessageEvent *ev);
  void contactlist_cb(ContactListEvent *ev);
  void icons_changed_cb();

  static gint sort_func( GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);

  // signals
  Signal1<void,unsigned int> user_popup;
  Signal1<void,unsigned int> userinfo;
};

#endif
