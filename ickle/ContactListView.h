/* $Id: ContactListView.h,v 1.26 2002-10-30 23:35:10 barnabygray Exp $
 *
 * Well actually it's a tree now.. :-)
 *
 * Copyright (C) 2001, 2002 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#include <gtk--/ctree.h>
#include <gtk--/menu.h>
#include <gtk--/pixmap.h>

#include <sigc++/signal_system.h>
#include <string>

#include "main.h"
#include <libicq2000/ContactTree.h>
#include <libicq2000/Contact.h>

#include "Icons.h"
#include "MessageQueue.h"

using SigC::Signal1;

class ContactListView : public Gtk::CTree {
 private:
  typedef Gtk::CTree_Helpers::RowIterator citerator;
  typedef Gtk::CTree_Helpers::TreeList::iterator titerator;

  struct RowData 
  {
    enum { Contact, Group } type;

    unsigned short group_id;
    
    unsigned int uin;
    ICQ2000::Status status;
    unsigned int msgs;
    std::string alias;
  };
  
  Gtk::Menu rc_popup_contact, rc_popup_group, rc_popup_blank;
  Gtk::MenuItem *rc_popup_away, *rc_popup_auth;

  MessageQueue& m_message_queue;
  class IckleGUI& m_gui;

  SigC::Connection contactlist_conn;

  void update_row(const ICQ2000::ContactRef& c);

  titerator lookup_uin(unsigned int uin);
  Gtk::CTree_Helpers::RowIterator get_tree_group( ICQ2000::ContactTree::Group& gp );

  // contact rc callbacks
  void userinfo_cb();
  void remove_contact_cb();
  void fetch_away_msg_cb();
  void send_auth_req_cb();

  // group rc callbacks
  void group_rename_cb();
  void group_remove_cb();

  // blank rc callbacks
  void blank_add_group_cb();

  unsigned int get_selection_contact_uin();
  ICQ2000::ContactTree::Group* get_selection_group();

 protected:
  //  virtual gint key_press_event_impl(GdkEventKey* ev);

  static void tree_move_cfunc(GtkCTree *ctree, GtkCTreeNode *child, GtkCTreeNode *parent,
			      GtkCTreeNode *sibling, gpointer data);
  void tree_move_impl(GtkCTree *ctree, GtkCTreeNode *child, GtkCTreeNode *parent,
		      GtkCTreeNode *sibling, gpointer data);

 private:
  bool m_single_click, m_check_away_click;
  int m_sort;

 public:
  ContactListView(IckleGUI& gui, MessageQueue& mq);
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
  static int status_order(ICQ2000::Status);

  static gint sort_func( GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
  static gboolean drag_compare_func( GtkCTree *clist,
				     GtkCTreeNode *source_node,
				     GtkCTreeNode *new_parent,
				     GtkCTreeNode *new_sibling);
  
  // signals
  SigC::Signal1<void,unsigned int> user_popup;
  SigC::Signal1<void,unsigned int> userinfo;
};

#endif
