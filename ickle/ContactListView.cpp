/* $Id: ContactListView.cpp,v 1.28 2002-01-26 14:24:24 barnabygray Exp $
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

#include "ContactListView.h"

#include <gdk/gdkkeysyms.h>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include "sstream_fix.h"

#include <libicq2000/Client.h>
#include "PromptDialog.h"

#include "main.h"
#include "Settings.h"

using std::string;
using std::vector;
using std::transform;
using std::inserter;
using std::ostringstream;

ContactListView::ContactListView()
  : CList(2), m_single_click(false),
    m_check_away_click(false),
    m_sort(SORT_MESSAGES_STATUS)
{
  column(0).set_title("S");
  column(0).set_width(15);
  column(0).set_justification(GTK_JUSTIFY_CENTER);
  column(0).set_resizable(false);
  column(1).set_title("Alias");
  column(1).set_resizable(false);
  column_titles_show();

  // callbacks
  icqclient.contactlist.connect(slot(this,&ContactListView::contactlist_cb));
  g_icons.icons_changed.connect(slot(this,&ContactListView::icons_changed_cb));
  g_settings.settings_changed.connect(slot(this,&ContactListView::settings_changed_cb));

  set_selection_mode(GTK_SELECTION_BROWSE);
  set_row_height(18);

  set_sort_column(0);
  set_auto_sort(false);
  set_compare_func(&ContactListView::sort_func);
  set_user_data (this);

  button_press_event.connect(slot(this,&ContactListView::button_press_cb));

   // Make a popup window
   {
     using namespace Gtk::Menu_Helpers;
     MenuList& ml = rc_popup.items();
     ml.push_back( MenuElem( "Check away message", slot( this, &ContactListView::fetch_away_msg_cb ) ) );
     ml.push_back( MenuElem( "User Info", slot( this, &ContactListView::userinfo_cb ) ) );
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

void ContactListView::setSingleClick(bool b) 
{
  m_single_click = b;
}

void ContactListView::setCheckAwayClick(bool b)
{
  m_check_away_click = b;
}

void ContactListView::click_column_impl(gint c)
{
  if (c == 0) {
    if (m_sort == SORT_MESSAGES_STATUS) m_sort = SORT_STATUS_MESSAGES;
                                   else m_sort = SORT_MESSAGES_STATUS;
  }
  else if (c == 1) {
    m_sort = SORT_ALIAS;
  }
  sort();
}

gint ContactListView::sort_func( GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2 )
{
  RowData *d1 = (RowData*)((const GtkCListRow*)ptr1)->data;
  RowData *d2 = (RowData*)((const GtkCListRow*)ptr2)->data;

  int s = (d1->status < d2->status) ? -1 : (d1->status > d2->status) ? 1 : 0;
  int m = (d1->msgs > d2->msgs) ? -1 : (d1->msgs < d2->msgs) ? 1 : 0;
  int a = d1->alias.compare(d2->alias);

  ContactListView * clist_obj = ((ContactListView*)gtk_object_get_user_data((GtkObject*)clist));

  switch (clist_obj->m_sort) {
    case SORT_MESSAGES_STATUS: return m ? m : s ? s : a;
    case SORT_STATUS_MESSAGES: return s ? s : m ? m : a;
    case SORT_ALIAS: return a ? a : m ? m : s;
  }
  return 0;
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

// Try to move selection to the next contact that starts with the letter
// on a keypress event.
gint ContactListView::key_press_event_impl(GdkEventKey *ev) {
  char key = tolower(ev->string[0]);

  if (rows().size() == 0) return Gtk::CList::key_press_event_impl(ev);

  // Start from the currently selected row
  RowIterator row_iter, start_iter;

  Gtk::CList_Helpers::SelectionList& sl = selection();
  if (sl.begin() != sl.end()) {
    row_iter = rows().begin();
    int n = sl.front().get_row_num();
    while (--n >= 0) { ++row_iter; }
    // there's no better way to convert a SelectionIterator -> RowIterator :-(
  } else {
    row_iter = rows().begin();
  }

  if( ev->keyval == GDK_Return || ev->keyval== GDK_KP_Enter || ev->keyval== GDK_space ) {
    user_popup.emit( ((RowData *) row_iter->get_data() )->uin );
  }
  else {
    start_iter = row_iter;
    ++row_iter;
    if (row_iter == rows().end()) row_iter = rows().begin();

    while (row_iter != start_iter) {
      Row &row = *row_iter;
      if (key == ((RowData *) row.get_data() )->alias[0]) {
        row.select();
        break;
      }
      ++row_iter;
      if (row_iter == rows().end()) row_iter = rows().begin();
    }

    if (!row_is_visible((*row_iter).get_row_num()))
      moveto( (*row_iter).get_row_num(), 0 );
  }
  return Gtk::CList::key_press_event_impl(ev);
}

void ContactListView::userinfo_cb() {
  userinfo.emit( current_selection_uin() );
}

void ContactListView::remove_user_cb() {
  unsigned int uin = current_selection_uin();
  Contact *c = icqclient.getContact(uin);
  if (c != NULL) {
    ostringstream ostr;
    ostr << "Are you sure you want to remove " << c->getAlias() << " (" << uin << ")?";
    PromptDialog p(PromptDialog::PROMPT_CONFIRM, ostr.str());
    if (p.run()) icqclient.removeContact(uin);
  }
}

void ContactListView::fetch_away_msg_cb() {
  Contact *c = icqclient.getContact( current_selection_uin() );
  if (c != NULL && c->getStatus() != STATUS_ONLINE && c->getStatus() != STATUS_OFFLINE)
    icqclient.SendEvent( new AwayMessageEvent(c) );
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

  if (rw == -1) return false;

  RowData *p = (RowData*)get_row_data(rw);
  Contact *c;
    
  if (ev->button == 3) {
    row(rw).select();
    rc_popup.popup(ev->button, ev->time);
    return true;
  }

  if (ev->button == 1) {

    if (ev->type == GDK_2BUTTON_PRESS) {
      user_popup.emit(p->uin);
      return true;
    }

    if (col == 0 && m_check_away_click == true) {
      c = icqclient.getContact( p->uin );
      if (c != NULL && c->getStatus() != STATUS_ONLINE && c->getStatus() != STATUS_OFFLINE)
	icqclient.SendEvent( new AwayMessageEvent(c) );

      return true;
    }

    if (m_single_click) {
      user_popup.emit(p->uin);
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
  string alias = c.getAlias();
  if (alias.empty()) {
    alias = c.getFirstName() + " " + c.getLastName();
    if (alias == " ") {
      if (c.isICQContact()) alias = c.getStringUIN();
      else alias = c.getMobileNo();
    }
  }
  transform( alias.begin(), alias.end(), inserter(rp->alias, rp->alias.begin()), tolower );
  (*row)[1].set_text( alias );
}

void ContactListView::contactlist_cb(ContactListEvent *ev) {
  Contact *c = ev->getContact();
  unsigned int uin = c->getUIN();

  if (ev->getType() == ContactListEvent::UserAdded) {
    vector<string> a;
    a.push_back("");
    a.push_back("");

    citerator cr = rows().insert(rows().end(),a);

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

void ContactListView::settings_changed_cb(const string& key) {
  if (key == "mouse_single_click")
    m_single_click = g_settings.getValueBool(key);
  else if (key == "mouse_check_away_click")
    m_check_away_click = g_settings.getValueBool(key);
}
