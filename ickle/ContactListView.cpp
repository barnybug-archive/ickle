/* $Id: ContactListView.cpp,v 1.39 2002-04-28 23:38:11 barnabygray Exp $
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

#include "IckleGUI.h"
#include "PromptDialog.h"
#include "SendAuthReqDialog.h"

#include "main.h"
#include "Settings.h"

using std::string;
using std::vector;
using std::transform;
using std::inserter;
using std::ostringstream;

using ICQ2000::ContactRef;

ContactListView::ContactListView(IckleGUI& gui, MessageQueue& mq)
  : CList(2),
    m_single_click(false),
    m_check_away_click(false),
    m_gui(gui),
    m_message_queue(mq)
{
  column(0).set_title("S");
  column(0).set_width(15);
  column(0).set_justification(GTK_JUSTIFY_CENTER);
  column(0).set_resizable(false);
  column(1).set_title("Alias");
  column(1).set_resizable(false);
  column_titles_show();

  // -- library callbacks      --
  icqclient.contactlist.connect(slot(this,&ContactListView::contactlist_cb));
  icqclient.contact_status_change_signal.connect(slot(this,&ContactListView::contact_status_change_cb));
  icqclient.contact_userinfo_change_signal.connect(slot(this,&ContactListView::contact_userinfo_change_cb));

  // -- MessageQueue callbacks --
  m_message_queue.added.connect(slot(this, &ContactListView::queue_added_cb));
  m_message_queue.removed.connect(slot(this, &ContactListView::queue_removed_cb));

  // -- gui callbacks          --
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
     rc_popup_away = ml.back();
     ml.push_back( MenuElem( "User Info", slot( this, &ContactListView::userinfo_cb ) ) );
     ml.push_back( MenuElem( "Send Auth Request", slot( this, &ContactListView::send_auth_req_cb ) ) );
     rc_popup_auth = ml.back();
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
  m_sort = c;
  g_settings.setValue ("sort_contact_list_column", c);
  sort();
}

void ContactListView::load_sort_column ()
{
  m_sort = g_settings.getValueInt ("sort_contact_list_column");
}

gint ContactListView::sort_func( GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2 )
{
  RowData *d1 = (RowData*)((const GtkCListRow*)ptr1)->data;
  RowData *d2 = (RowData*)((const GtkCListRow*)ptr2)->data;

  int o1 = status_order (d1->status);
  int o2 = status_order (d2->status);

  int s = (o1 < o2) ? -1 : (o1 > o2) ? 1 : 0;
  int m = (d1->msgs > d2->msgs) ? -1 : (d1->msgs < d2->msgs) ? 1 : 0;
  int a = d1->alias.compare(d2->alias);

  ContactListView * clist_obj = ((ContactListView*)gtk_object_get_user_data((GtkObject*)clist));

  switch (clist_obj->m_sort) {
    case 0: return m ? m : s ? s : a;
    case 1: return a ? a : m ? m : s;
  }
  return 0;
}

int ContactListView::status_order (ICQ2000::Status s)
{
  switch (s) {
    case ICQ2000::STATUS_ONLINE:       return 1;
    case ICQ2000::STATUS_FREEFORCHAT:  return 2;
    case ICQ2000::STATUS_OCCUPIED:     return 3;
    case ICQ2000::STATUS_DND:          return 4;
    case ICQ2000::STATUS_AWAY:         return 5;
    case ICQ2000::STATUS_NA:           return 6;
    case ICQ2000::STATUS_OFFLINE:      return 7;
  }
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
  else if ( ev->keyval == GDK_Home ) {
    row(0).select();
    moveto(0, 0);
  }
  else if ( ev->keyval == GDK_End ) {
    row( rows().size()-1 ).select();
    moveto( rows().size()-1, 0 );
  }
  else if ( isalnum(key) && !(ev->state & GDK_CONTROL_MASK) && !(ev->state & GDK_MOD1_MASK)) {
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
  ContactRef c = icqclient.getContact(uin);
  if (c.get() != NULL) {
    ostringstream ostr;
    ostr << "Are you sure you want to remove " << c->getAlias();
    if (c->isICQContact()) ostr << " (" << uin << ")";
    ostr << "?";
    PromptDialog p(&m_gui, PromptDialog::PROMPT_CONFIRM, ostr.str());
    if (p.run()) icqclient.removeContact(uin);
  }
}

void ContactListView::fetch_away_msg_cb() {
  ContactRef c = icqclient.getContact( current_selection_uin() );
  if (c.get() != NULL
      && c->getStatus() != ICQ2000::STATUS_ONLINE
      && c->getStatus() != ICQ2000::STATUS_OFFLINE)
    icqclient.SendEvent( new ICQ2000::AwayMessageEvent(c) );
}

void ContactListView::send_auth_req_cb() {
  ContactRef c = icqclient.getContact( current_selection_uin() );
  if (c.get() != NULL && c->isICQContact()) {
    SendAuthReqDialog *dialog = new SendAuthReqDialog(&m_gui, c);
    manage( dialog );
  }
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
  ContactRef c = icqclient.getContact(p->uin);
    
  if (ev->button == 3) {
    row(rw).select();
    rc_popup_away->set_sensitive(c.get() != NULL &&
                             c->getStatus() != ICQ2000::STATUS_ONLINE &&
                             c->getStatus() != ICQ2000::STATUS_OFFLINE);
    rc_popup_auth->set_sensitive(icqclient.isConnected());

    rc_popup.popup(ev->button, ev->time);
    return true;
  }

  if (ev->button == 1) {

    if (ev->type == GDK_2BUTTON_PRESS) {
      user_popup.emit(p->uin);
      return true;
    }

    if (col == 0 && m_check_away_click == true) {
      if (c.get() != NULL
	  && c->getStatus() != ICQ2000::STATUS_ONLINE
	  && c->getStatus() != ICQ2000::STATUS_OFFLINE)
	icqclient.SendEvent( new ICQ2000::AwayMessageEvent(c) );

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

void ContactListView::update_row(const ContactRef& c) {

  citerator row = lookupUIN(c->getUIN());
  if (row == rows().end()) return;

  RowData *rp = (RowData*)(*row).get_data();
  rp->status = c->getStatus();
  rp->msgs = 0;
  ImageLoader *p;

  rp->msgs = m_message_queue.get_contact_size(c);

  if (rp->msgs > 0) {

    MessageEvent *ev = m_message_queue.get_contact_first_message(c);
    if (ev->getServiceType() == MessageEvent::ICQ) {
      ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
      p = g_icons.IconForEvent(icq->getICQMessageType());
    }
    
  } else {
    p = g_icons.IconForStatus(c->getStatus(),c->isInvisible());
  }
  
  (*row)[0].set_pixmap( p->pix(), p->bit() );

  string alias = c->getNameAlias();
  transform( alias.begin(), alias.end(), inserter(rp->alias, rp->alias.begin()), tolower );
  (*row)[1].set_text( alias );
}

void ContactListView::contactlist_cb(ICQ2000::ContactListEvent *ev) {
  ContactRef c = ev->getContact();
  unsigned int uin = c->getUIN();

  if (ev->getType() == ICQ2000::ContactListEvent::UserAdded) {
    vector<string> a;
    a.push_back("");
    a.push_back("");

    citerator cr = rows().insert(rows().end(),a);

    RowData *p = new RowData;
    p->uin = c->getUIN();
    (*cr).set_data(p);

    update_row(c);

    columns_autosize();
    sort();

  } else if (ev->getType() == ICQ2000::ContactListEvent::UserRemoved) {

    citerator cr = lookupUIN(uin);
    RowData *p = (RowData*)(*cr).get_data();
    delete p;
    rows().erase(cr);
    columns_autosize();
    sort();
  }
}

void ContactListView::contact_userinfo_change_cb(ICQ2000::UserInfoChangeEvent *ev)
{
  update_row(ev->getContact());
  sort();
}

void ContactListView::contact_status_change_cb(ICQ2000::StatusChangeEvent *ev)
{
  update_row(ev->getContact());
  sort();
}

void ContactListView::icons_changed_cb() {
  citerator curr = rows().begin();
  while (curr != rows().end()) {
    RowData *rp = (RowData*)(*curr).get_data();
    ContactRef c = icqclient.getContact( rp->uin );
    if (c.get() != NULL) update_row(c);
    ++curr;
  }
}

void ContactListView::settings_changed_cb(const string& key) {
  if (key == "mouse_single_click")
    m_single_click = g_settings.getValueBool(key);
  else if (key == "mouse_check_away_click")
    m_check_away_click = g_settings.getValueBool(key);
}

void ContactListView::queue_added_cb(MessageEvent *ev)
{
  if (ev->getServiceType() != MessageEvent::ICQ) return;
  ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
  update_row(icq->getICQContact());
  sort();
}

void ContactListView::queue_removed_cb(MessageEvent *ev)
{
  if (ev->getServiceType() != MessageEvent::ICQ) return;
  ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
  update_row(icq->getICQContact());
  sort();
}
