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

#include "ContactListView.h"

#include <vector>
#include <string>
#include <utility>

#include "Client.h"

using std::string;
using std::vector;

ContactListView::ContactListView()
  : CList(2) {
  set_column_title(0, "S");
  set_column_min_width(0, 15);
  set_column_title(1, "Alias");
  column_titles_show();
  
  // callbacks
  icqclient.contactlist.connect(slot(this,&ContactListView::contactlist_cb));

  set_selection_mode(GTK_SELECTION_BROWSE);
  set_row_height(18);

  set_sort_column(0);
  set_compare_func(&ContactListView::sort_func);

  button_press_event.connect(slot(this,&ContactListView::button_press_cb));

   // Make a popup window
   {
     using namespace Gtk::Menu_Helpers;
     MenuList& ml = rc_popup.items();
     ml.push_back( MenuElem( "Check away message", slot( this, &ContactListView::fetch_away_msg_cb ) ) );
     ml.push_back( MenuElem( "User Info", slot( this, &ContactListView::user_info_cb ) ) );
     ml.push_back( MenuElem( "Remove User", slot( this, &ContactListView::remove_user_cb ) ) );
   }
}

ContactListView::~ContactListView() {
  clear();
}

void ContactListView::setupAccelerators() {
  // the popup menu needs to be told where to place its accelerators
  rc_popup.accelerate( *(this->get_toplevel()) );
}

gint ContactListView::sort_func( GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2 ) {
  const GtkCListRow *row1 = (const GtkCListRow *) ptr1;
  const GtkCListRow *row2 = (const GtkCListRow *) ptr2;

  // get data
  RowData *d1 = (RowData*)row1->data;
  RowData *d2 = (RowData*)row2->data;

  Status s1 = d1->status;
  Status s2 = d2->status;
  unsigned int m1 = d1->msgs;
  unsigned int m2 = d2->msgs;

  /* contacts are sorted by status, then by
   * number of messages - this is just my personal
   * preference, maybe at some date make it more
   * customisable
   */
  
  return ((s1 < s2)
	  ? -1
	  : (s1 > s2)
	  ? 1
	  : // s1 == s2
	  (m1 > m2)
	  ? -1
	  : (m2 > m1)
	  ? 1
	  : 0
	  );
}

void ContactListView::clear() {
  RowData *p;
  while (!rows().empty()) {
    p = (RowData*)row(0).get_data();
    delete p;
    rows().remove(row(0));
  }
  Gtk::CList::clear();
}

void ContactListView::user_info_cb() {
  user_info.emit( current_selection_uin() );
}

void ContactListView::remove_user_cb() {
  Gtk::CList_Helpers::SelectionList sl = selection();
  Row r = *(sl.begin());
  RowData *p = (RowData*)(*r).get_data();
  icqclient.removeContact(p->uin);
}

void ContactListView::fetch_away_msg_cb() {
  Contact *c = icqclient.getContact( current_selection_uin() );
  if (c != NULL) icqclient.fetchAwayMsg(c);
}

unsigned int ContactListView::current_selection_uin() {
  Gtk::CList_Helpers::SelectionList sl = selection();
  if (sl.begin() == sl.end()) return 0;
  Row r = *(sl.begin());
  RowData *p = (RowData*)(*r).get_data();
  return p->uin;
}

gint ContactListView::button_press_cb(GdkEventButton *ev) {
  int rw = -1, col;

  get_selection_info((int)ev->x, (int)ev->y, &rw, &col);
  if (rw != -1) {
    RowData *p = (RowData*)get_row_data(rw);
    if (ev->type == GDK_2BUTTON_PRESS && ev->button == 1) {
      user_popup.emit(p->uin);
      return true;
    } else if (ev->button == 1 && col == 0) {
      Contact *c = icqclient.getContact( p->uin );
      if (c != NULL) icqclient.fetchAwayMsg(c);
      row(rw).select();
      return true;
    } else if(ev->button == 3) {
      row(rw).select();
      rc_popup.popup(ev->button, ev->time);
      return true;
    }
  }
  return false;
}

ContactListView::citerator ContactListView::lookupUIN(unsigned int uin) {
  citerator curr = rows().begin();
  while (curr != rows().end()) {
    RowData *p = (RowData*)((*curr).get_data());
    if (p->uin == uin) return curr;
    ++curr;
  }
  return rows().end();
}

void ContactListView::UpdateRow(const Contact& c) {

  citerator row = lookupUIN(c.getUIN());
  if (row == rows().end()) return;

  RowData *rp = (RowData*)(*row).get_data();
  rp->status = c.getStatus();
  rp->msgs = c.numberPendingMessages();

  ImageLoader *p;
  if (rp->msgs > 0) {
    MessageEvent *ev = c.getPendingMessage();
    p = g_icons.IconForEvent(ev->getType());
  } else {
    p = g_icons.IconForStatus(c.getStatus(),c.isInvisible());
  }
  
  (*row)[0].set_pixmap( p->pix(), p->bit() );
  (*row)[1].set_text( c.getAlias() );
}

void ContactListView::contactlist_cb(ContactListEvent *ev) {
  Contact *c = ev->getContact();
  unsigned int uin = c->getUIN();

  if (ev->getType() == ContactListEvent::UserAdded) {
    vector<string> a;
    a.push_back("");
    a.push_back("");

    citerator cr = rows().insert(rows().end(),a);
    //    m_row_map.insert(  pair<unsigned int, citerator>( c->getUIN() , cr )  );

    RowData *p = new RowData;
    p->uin = c->getUIN();
    (*cr).set_data(p);

    UpdateRow(*c);

    columns_autosize();
    sort();

  } else if (ev->getType() == ContactListEvent::UserRemoved) {

    citerator cr = lookupUIN(uin);
    RowData *p = (RowData*)(*cr).get_data();
    delete p;
    rows().erase(cr);
    //    m_row_map.erase(uin);
    sort();

  } else if (ev->getType() == ContactListEvent::MessageQueueChanged
	  || ev->getType() == ContactListEvent::StatusChange 
	  || ev->getType() == ContactListEvent::UserInfoChange) {

    UpdateRow(*c);
    sort();
  }
}

bool ContactListView::message_cb(MessageEvent *ev) {
  Contact *c = ev->getContact();

  UpdateRow(*c);
  sort();

  return false; // return false because the message shouldn't be swallowed yet
}

void ContactListView::icons_changed_cb() {
  citerator curr = rows().begin();
  while (curr != rows().end()) {
    RowData *rp = (RowData*)(*curr).get_data();
    Contact *c = icqclient.getContact( rp->uin );
    if (c != NULL) UpdateRow(*c);
    ++curr;
  }
}
