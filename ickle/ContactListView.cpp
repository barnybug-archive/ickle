/* $Id: ContactListView.cpp,v 1.44 2002-10-30 22:09:37 barnabygray Exp $
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

#include "AddGroupDialog.h"
#include "RenameGroupDialog.h"
#include "RemoveGroupDialog.h"
#include "RemoveContactDialog.h"

#include "main.h"
#include "Settings.h"

using std::string;
using std::vector;
using std::transform;
using std::inserter;
using std::ostringstream;

using ICQ2000::ContactRef;

ContactListView::ContactListView(IckleGUI& gui, MessageQueue& mq)
  : CTree(1, 0),
    m_message_queue(mq),
    m_gui(gui),
    m_single_click(false),
    m_check_away_click(false)
{
  set_show_stub(false);
  set_line_style(GTK_CTREE_LINES_NONE);
  set_expander_style(GTK_CTREE_EXPANDER_NONE);
  set_indent(2);
  set_spacing(0);
  set_reorderable(1);
  set_drag_compare_func(&ContactListView::drag_compare_func);

  // tree_move signal was never wrapped in gtkmm it seems..
  gtk_signal_connect(GTK_OBJECT(gtkobj()), "tree_move",
		     GTK_SIGNAL_FUNC(tree_move_cfunc), this);

  // -- library callbacks      --
  contactlist_conn = icqclient.contactlist.connect(slot(this,&ContactListView::contactlist_cb));
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

   {
     using namespace Gtk::Menu_Helpers;
     // The popup windows

     // popup for right-click contact
     MenuList& ml_c = rc_popup_contact.items();
     ml_c.push_back( MenuElem( "Check away message", slot( this, &ContactListView::fetch_away_msg_cb ) ) );
     rc_popup_away = ml_c.back();
     ml_c.push_back( MenuElem( "User Info", slot( this, &ContactListView::userinfo_cb ) ) );
     ml_c.push_back( MenuElem( "Send Auth Request", slot( this, &ContactListView::send_auth_req_cb ) ) );
     rc_popup_auth = ml_c.back();
     ml_c.push_back( MenuElem( "Remove Contact", slot( this, &ContactListView::remove_contact_cb ) ) );
     ml_c.push_back( MenuElem( "Add New Group", slot( this, &ContactListView::blank_add_group_cb ) ) );

     // popup for right-click group
     MenuList& ml_g = rc_popup_group.items();
     ml_g.push_back( MenuElem( "Rename Group", slot( this, &ContactListView::group_rename_cb ) ) );
     ml_g.push_back( MenuElem( "Remove Group", slot( this, &ContactListView::group_remove_cb ) ) );
     ml_g.push_back( MenuElem( "Add New Group", slot( this, &ContactListView::blank_add_group_cb ) ) );

     // popup for right-click over blank area
     MenuList& ml_b = rc_popup_blank.items();
     ml_b.push_back( MenuElem( "Add New Group", slot( this, &ContactListView::blank_add_group_cb ) ) );
     // anything else?
   }
}

ContactListView::~ContactListView() {
  clear();
}

void ContactListView::setupAccelerators() {
  // the popup menus need to be told where to place their accelerators
  rc_popup_contact.accelerate( *(this->get_toplevel()) );
  rc_popup_group.accelerate( *(this->get_toplevel()) );
  rc_popup_blank.accelerate( *(this->get_toplevel()) );
}

void ContactListView::setSingleClick(bool b) 
{
  m_single_click = b;
}

void ContactListView::setCheckAwayClick(bool b)
{
  m_check_away_click = b;
}

void ContactListView::load_sort_column ()
{
  m_sort = g_settings.getValueInt ("sort_contact_list_column");
}

// ughh.. all nasty Gtk stuff :-(
gboolean ContactListView::drag_compare_func( GtkCTree *ctree,
					     GtkCTreeNode *source_node,
					     GtkCTreeNode *new_parent,
					     GtkCTreeNode *new_sibling)
{
  RowData *rd = (RowData*)gtk_ctree_node_get_row_data(GTK_CTREE(ctree), source_node);
  if (rd->type == RowData::Contact) {
    if (new_parent) {
      rd = (RowData*)gtk_ctree_node_get_row_data(GTK_CTREE(ctree), new_parent);
      // allow contact to move into new group by dragging onto group title
      if (rd->type == RowData::Group)
	return TRUE; // allow contacts to move between groups
    }
  } else {
    if (new_parent)
      return FALSE; // don't allow groups to be moved into groups

    return TRUE; // allow groups to move up/down
  }
  return FALSE; // don't allow anything else
}

void ContactListView::tree_move_cfunc(GtkCTree *ctree, GtkCTreeNode *child, GtkCTreeNode *parent,
				      GtkCTreeNode *sibling, gpointer data)
{
  ((ContactListView *)data)->tree_move_impl(ctree, child, parent, sibling, data);
}

void ContactListView::tree_move_impl(GtkCTree *ctree, GtkCTreeNode *child, GtkCTreeNode *parent,
				     GtkCTreeNode *sibling, gpointer data) 
{
  RowData *rd_c = (RowData*)gtk_ctree_node_get_row_data(GTK_CTREE(ctree), child);

  if (rd_c->type == RowData::Contact) {
    RowData *rd_p = (RowData*)gtk_ctree_node_get_row_data(GTK_CTREE(ctree), parent);

    // deaden contact list signals - gtk handles the moving for us
    contactlist_conn.disconnect();

    ICQ2000::ContactTree& ct = icqclient.getContactTree();
    ICQ2000::ContactRef c = ct[ rd_c->uin ];
    ICQ2000::ContactTree::Group& from = ct.lookup_group_containing_contact( c );
    ICQ2000::ContactTree::Group& to = ct.lookup_group( rd_p->group_id );
    ct.relocate_contact( c, from, to );
  
    // safe to resuscitate contact list signals
    contactlist_conn = icqclient.contactlist.connect(slot(this,&ContactListView::contactlist_cb));
  }
}

gint ContactListView::sort_func( GtkCList *ctree, gconstpointer ptr1, gconstpointer ptr2 )
{
  RowData *d1 = (RowData*)((const GtkCListRow*)ptr1)->data;
  RowData *d2 = (RowData*)((const GtkCListRow*)ptr2)->data;

  int o1 = status_order (d1->status);
  int o2 = status_order (d2->status);

  int s = (o1 < o2) ? -1 : (o1 > o2) ? 1 : 0;
  int m = (d1->msgs > d2->msgs) ? -1 : (d1->msgs < d2->msgs) ? 1 : 0;
  int a = d1->alias.compare(d2->alias);

  ContactListView * clist_obj = ((ContactListView*)gtk_object_get_user_data((GtkObject*)ctree));

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
  return 0;
}

void ContactListView::clear() {
  RowData *p;
  while (!rows().empty()) {
    p = (RowData*)row(0).get_data();
    delete p;
    rows().remove(row(0));
  }
  Gtk::CTree::clear();
}

// Try to move selection to the next contact that starts with the letter
// on a keypress event.
/* TODO!
gint ContactListView::key_press_event_impl(GdkEventKey *ev) {
  char key = tolower(ev->string[0]);

  if (rows().size() == 0) return Gtk::CTree::key_press_event_impl(ev);

  // Start from the currently selected row
  Gtk::CTree_Helpers::RowIterator row_iter, start_iter;

  Gtk::CTree_Helpers::SelectionList& sl = selection();
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
  return Gtk::CTree::key_press_event_impl(ev);
}
*/

void ContactListView::userinfo_cb() {
  userinfo.emit( get_selection_contact_uin() );
}

void ContactListView::remove_contact_cb() {
  unsigned int uin = get_selection_contact_uin();
  ContactRef c = icqclient.getContact(uin);
  manage( new RemoveContactDialog(&m_gui, c) );
}

void ContactListView::fetch_away_msg_cb()
{
  ContactRef c = icqclient.getContact( get_selection_contact_uin() );
  if (c.get() != NULL
      && c->getStatus() != ICQ2000::STATUS_ONLINE
      && c->getStatus() != ICQ2000::STATUS_OFFLINE)
    icqclient.SendEvent( new ICQ2000::AwayMessageEvent(c) );
}

void ContactListView::send_auth_req_cb()
{
  ContactRef c = icqclient.getContact( get_selection_contact_uin() );
  if (c.get() != NULL && c->isICQContact()) {
    SendAuthReqDialog *dialog = new SendAuthReqDialog(&m_gui, c);
    manage( dialog );
  }
}

unsigned int ContactListView::get_selection_contact_uin()
{
  Gtk::CTree_Helpers::SelectionList sl = selection();
  if (sl.begin() == sl.end()) return 0;
  Row r = *(sl.begin());
  RowData *p = (RowData*)r.get_data();
  return p->uin;
}

ICQ2000::ContactTree::Group* ContactListView::get_selection_group()
{
  Gtk::CTree_Helpers::SelectionList sl = selection();
  if (sl.begin() == sl.end()) return NULL;
  Row r = *(sl.begin());
  RowData *p = (RowData*)r.get_data();
  if (p->type != RowData::Group) return NULL;
  return &(icqclient.getContactTree().lookup_group( p->group_id ));
}

gint ContactListView::button_press_cb(GdkEventButton *ev) {
  int rw = -1, col;
  get_selection_info((int)ev->x, (int)ev->y, &rw, &col);

  if (rw == -1) {
    // Click on blank area
    
    if (ev->button == 3) {
      rc_popup_blank.popup(ev->button, ev->time);
      return true;
    }
  } else {
    RowData *p = (RowData*)get_row_data(rw);

    if (p->type == RowData::Contact) {
      // Click on a Contact
      ContactRef c = icqclient.getContact(p->uin);
    
      if (ev->button == 3) {
	// Right-click
	row(rw).select();
	rc_popup_away->set_sensitive(c.get() != NULL &&
				     c->getStatus() != ICQ2000::STATUS_ONLINE &&
				     c->getStatus() != ICQ2000::STATUS_OFFLINE);
	rc_popup_auth->set_sensitive(icqclient.isConnected());

	rc_popup_contact.popup(ev->button, ev->time);
	return true;
      }

      if (ev->button == 1) {
	// Left-click
	if (ev->type == GDK_2BUTTON_PRESS) {
	  user_popup.emit(p->uin);
	  return true;
	}

	// TODO
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
    } else if (p->type == RowData::Group) {
      // Click on a Group

      if (ev->button == 3) {
	row(rw).select();
	rc_popup_group.popup(ev->button, ev->time);
	return true;
      }
    }
  }
  
  return false;  
}

ContactListView::citerator ContactListView::lookup_uin(unsigned int uin) {
  citerator curr = rows().begin();
  while (curr != rows().end()) {
    RowData *p = (RowData*)((*curr).get_data());
    if (p->type == RowData::Contact && p->uin == uin) return curr;
    ++curr;
  }
  return rows().end();
}

void ContactListView::update_row(const ContactRef& c) {

  citerator row = lookup_uin(c->getUIN());
  if (row == rows().end()) return;

  RowData *rp = (RowData*)(*row).get_data();
  rp->status = c->getStatus();
  rp->msgs = 0;
  Gtk::ImageLoader *p;

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
  
  (*row).set_opened( p->pix(), p->bit() );
  (*row).set_closed( p->pix(), p->bit() );

  string alias = c->getNameAlias();
  transform( alias.begin(), alias.end(), inserter(rp->alias, rp->alias.begin()), tolower );
  (*row)[0].set_text( alias );
}

Gtk::CTree_Helpers::RowIterator ContactListView::get_tree_group( ICQ2000::ContactTree::Group& gp )
{
  Gtk::CTree_Helpers::RowList::iterator iter = rows().begin();
  while (iter != rows().end()) {
    RowData *rd = (RowData*)(*iter).get_data();
    if (rd->type == RowData::Group && rd->group_id == gp.get_id()) return iter;
    ++iter;
  }
  return rows().end();
}

void ContactListView::contactlist_cb(ICQ2000::ContactListEvent *ev) {

  if (ev->getType() == ICQ2000::ContactListEvent::UserAdded) {
    
    ICQ2000::UserAddedEvent *cev = static_cast<ICQ2000::UserAddedEvent*>(ev);
    ContactRef c = cev->getContact();
    unsigned int uin = c->getUIN();
    
    vector<string> a;
    a.push_back("");

    Gtk::CTree_Helpers::RowIterator iter = get_tree_group( const_cast<ICQ2000::ContactTree::Group&>(cev->get_group()) );
    Row& parent = *iter;
    
    // create + set per row data
    RowData *p = new RowData;
    p->type = RowData::Contact;
    p->uin = c->getUIN();
    citerator cr = parent.subtree()
                         .insert( parent.subtree().end(),
				  Gtk::CTree_Helpers::Element( a, false, false ) );
    (*cr).set_data(p);

    update_row(c);

    columns_autosize();
    sort();

  } else if (ev->getType() == ICQ2000::ContactListEvent::UserRemoved) {
    ICQ2000::UserRemovedEvent *cev = static_cast<ICQ2000::UserRemovedEvent*>(ev);
    ContactRef c = cev->getContact();
    unsigned int uin = c->getUIN();
    
    // delete per row data
    citerator cr = lookup_uin(uin);
    RowData *p = (RowData*)(*cr).get_data();
    delete p;

    rows().erase(cr);
    columns_autosize();
    sort();
  } else if (ev->getType() == ICQ2000::ContactListEvent::GroupAdded) {
    ICQ2000::GroupAddedEvent *cev = static_cast<ICQ2000::GroupAddedEvent*>(ev);
    vector<string> a;
    a.push_back( cev->get_group().get_label() );
    citerator cr = rows().insert( rows().end(),
				  Gtk::CTree_Helpers::BranchElem( a, true ) );

    // create + set per row data
    RowData *p = new RowData;
    p->type = RowData::Group;
    p->group_id = cev->get_group().get_id();
    (*cr).set_data(p);

  } else if (ev->getType() == ICQ2000::ContactListEvent::GroupRemoved) {
    ICQ2000::GroupRemovedEvent *cev = static_cast<ICQ2000::GroupRemovedEvent*>(ev);

    Gtk::CTree_Helpers::RowIterator iter = get_tree_group( const_cast<ICQ2000::ContactTree::Group&>(cev->get_group()) );
    rows().remove( *iter );
    
  } else if (ev->getType() == ICQ2000::ContactListEvent::GroupChange) {
    ICQ2000::GroupChangeEvent *cev = static_cast<ICQ2000::GroupChangeEvent*>(ev);

    Gtk::CTree_Helpers::RowIterator iter = get_tree_group( const_cast<ICQ2000::ContactTree::Group&>(cev->get_group()) );
    Row& row = *iter;
    row[0].set_text( cev->get_group().get_label() );
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

void ContactListView::group_rename_cb()
{
  ICQ2000::ContactTree::Group *gp = get_selection_group();
  if (gp != NULL)
    manage( new RenameGroupDialog(&m_gui, gp) );
}

void ContactListView::group_remove_cb()
{
  ICQ2000::ContactTree::Group *gp = get_selection_group();
  if (gp != NULL)
    manage( new RemoveGroupDialog(&m_gui, gp) );
}

void ContactListView::blank_add_group_cb()
{
  manage( new AddGroupDialog(&m_gui) );
}
