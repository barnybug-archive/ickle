/* $Id: ContactListView.h,v 1.40 2004-02-08 20:23:10 cborni Exp $
 *
 * ContactList(Tree)View
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

#include <map>

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/menu.h>

#include <libicq2000/events.h>

#include "MessageQueue.h"

class ContactListView : public Gtk::TreeView,
	                public sigslot::has_slots<>
{
 private:
  Gtk::Window& m_parent;

  MessageQueue& m_message_queue;

  // --- callbacks           ----

  // -- library callbacks      --
  void contactlist_cb(ICQ2000::ContactListEvent *ev);
  void contact_userinfo_change_cb(ICQ2000::UserInfoChangeEvent *ev);
  void contact_status_change_cb(ICQ2000::StatusChangeEvent *ev);
  void want_auto_resp_cb(ICQ2000::ICQMessageEvent *ev);

  // -- MessageQueue callbacks --
  void queue_added_cb(MessageEvent *ev);
  void queue_removed_cb(MessageEvent *ev);

  // -- gui callbacks          --
  void icons_changed_cb();
  void settings_changed_cb(const std::string&);

  // -- Gtk callbacks          --
  virtual bool on_button_press_event(GdkEventButton * event);
  virtual void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn * col);

  void add_group(const ICQ2000::ContactTree::Group& gp);
  void add_contact(const ICQ2000::ContactRef& c, const ICQ2000::ContactTree::Group& gp);
  void remove_group(const ICQ2000::ContactTree::Group& gp);
  void remove_contact(const ICQ2000::ContactRef& c);

  void update_contact(const ICQ2000::ContactRef& c);
  void update_icon(Gtk::TreeModel::Row& row, const ICQ2000::ContactRef& c);
  void update_group(const ICQ2000::ContactTree::Group& gp);
  void update_list();

  // right-click popup callbacks
  void contact_fetch_away_msg_cb();
  void contact_userinfo_cb();
  void contact_send_auth_req_cb();
  void contact_use_encoding_cb();
  void contact_send_file_cb();
  void contact_add_cb();
  void contact_remove_cb();
  void contact_move_to_group_cb(ICQ2000::ContactTree::Group * gp);
  void group_rename_cb();
  void group_remove_cb();
  void group_add_cb();
  void group_fetch_all_away_msg_cb();


  gint sort_func(const Gtk::TreeModel::iterator& iter1, const Gtk::TreeModel::iterator& iter2);
  int status_order (ICQ2000::Status s);

  bool contact_restore_weight_timeout_cb(ICQ2000::ContactRef c);
  bool contact_restore_colour_timeout_cb(ICQ2000::ContactRef c);

  void popup_contact_menu(guint button, guint32 activate_time, unsigned int uin);

  ICQ2000::ContactRef get_selected_contact();
  ICQ2000::ContactTree::Group * get_selected_group();

  // Tree model columns
  struct ModelColumns : public Gtk::TreeModelColumnRecord
  {
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > icon;
    Gtk::TreeModelColumn< Glib::ustring >  nick;
    Gtk::TreeModelColumn< Glib::ustring >  group_name;
    Gtk::TreeModelColumn< std::string >  nick_sort_key;
    Gtk::TreeModelColumn< bool > is_contact;
    Gtk::TreeModelColumn< bool > is_group;
    Gtk::TreeModelColumn< unsigned int > id;
    Gtk::TreeModelColumn< ICQ2000::Status > status;
    Gtk::TreeModelColumn< unsigned int > messages;
    Gtk::TreeModelColumn< bool > visible;
    Gtk::TreeModelColumn< int > text_weight;
    Gtk::TreeModelColumn< Gdk::Color > text_colour;
    Gtk::TreeModelColumn< Gdk::Color > bg_colour;
    Gtk::TreeModelColumn< Glib::ustring > numbers;
    Gtk::TreeModelColumn< unsigned int > total_online;
    Gtk::TreeModelColumn< unsigned int > total;

    ModelColumns()
    {
      add(icon);
      add(nick);
      add(group_name);
      add(nick_sort_key);
      add(is_contact);
      add(is_group);
      add(id);
      add(status);
      add(messages);
      add(visible);
      add(text_weight);
      add(text_colour);
      add(bg_colour);
      add(numbers);
      add(total_online);
      add(total);
    }
  };

  const ModelColumns m_columns;

  // the Tree store
  Glib::RefPtr<Gtk::TreeStore> m_reftreestore;

  // group mapping
  std::map< unsigned int, Gtk::TreeModel::iterator > m_group_map;

  // contact mapping
  std::map< unsigned int, Gtk::TreeModel::iterator > m_contact_map;

  bool m_offline_contacts;
  bool m_single_click;
  bool m_check_away_click;

  Gtk::Menu m_rc_popup_contact, m_rc_popup_group, m_rc_popup_blank;
  Gtk::Menu m_rc_groups_list;
  Gtk::ImageMenuItem * m_rc_popup_away;
  Gtk::MenuItem      * m_rc_popup_auth;
  Gtk::CheckMenuItem * m_rc_popup_encoding;
  SigC::Connection      m_rc_popup_encoding_conn;

  // signals
  SigC::Signal1<void, unsigned int> m_signal_messagebox_popup;
  SigC::Signal1<void, const ICQ2000::ContactRef&> m_signal_userinfo_popup;

 public:
  ContactListView(Gtk::Window& parent, MessageQueue& mq);
  ~ContactListView();

  void set_show_offline_contacts(bool b);

  // signal accessors
  SigC::Signal1<void, unsigned int>& signal_messagebox_popup();
  SigC::Signal1<void, const ICQ2000::ContactRef&>& signal_userinfo_popup();
};

#endif /* CONTACTLISTVIEW_H */
