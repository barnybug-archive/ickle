/*
 * SearchDialog
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * Ported from ickle-gtkmm-1.2 by Daniel Sundberg <sumpan@sumpan.com>
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

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/notebook.h>
#include <gtkmm/entry.h>
#include <gtkmm/combo.h>
#include <gtkmm/treeview.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodel.h>

#include <libicq2000/Client.h>
#include <libicq2000/events.h>
#include <libicq2000/userinfoconstants.h>

// TODO: add status icon to search results

class SearchDialog : public Gtk::Dialog, public sigslot::has_slots<>
{
 private:
  Gtk::Notebook m_notebook;

  // -- whitepage search --
  Gtk::Entry m_alias_entry, m_firstname_entry, m_lastname_entry;
  Gtk::Entry m_email_entry, m_city_entry, m_state_entry;
  Gtk::Entry m_company_name_entry, m_department_entry, m_position_entry;
  Gtk::CheckButton m_only_online_check;
  Gtk::OptionMenu m_sex_menu, m_agerange_menu;
  ICQ2000::Sex m_sex_selected;
  ICQ2000::AgeRange m_agerange_selected;
  Gtk::Combo m_language_combo, m_country_combo;
  Gtk::Combo m_group_list;

  // -- uin search --
  Gtk::Entry m_uin_entry;

  // -- keyword search --
  Gtk::Entry m_keyword_entry;

  Gtk::TreeView m_treeview;
  Glib::RefPtr<Gtk::ListStore> m_ref_liststore;

  struct ModelColumns : public Gtk::TreeModelColumnRecord
  {
    Gtk::TreeModelColumn<Glib::ustring> alias;
    Gtk::TreeModelColumn<unsigned int>           uin;
    Gtk::TreeModelColumn<Glib::ustring> first_name;
    Gtk::TreeModelColumn<Glib::ustring> last_name;
    Gtk::TreeModelColumn<Glib::ustring> email;
    Gtk::TreeModelColumn<bool>          auth_req;
    Gtk::TreeModelColumn<ICQ2000::ContactRef> contact;
    
    ModelColumns() { add(alias); add(uin); add(first_name);
      add(last_name); add(email); add(auth_req); }
  };
  ModelColumns m_columns;

  Gtk::Button m_ok_button, m_search_button, m_stop_button, m_add_button, m_reset_button;
  Gtk::Statusbar m_status;
  guint m_status_context;

  bool m_in_progress;
  ICQ2000::SearchResultEvent *m_ev;

  static void clist_data_destroy_cb(gpointer data);
  void selected_group_cb(ICQ2000::ContactTree::Group * gp);

  std::map< unsigned int, Gtk::ComboDropDownItem * > m_group_map;
  ICQ2000::ContactTree::Group * m_selected_group;

 protected:

  // -- GUI button callbacks --
  void ok_cb();
  void search_cb();
  void stop_cb();
  void add_cb();
  void reset_cb();

  void select_row_cb();
  void unselect_row_cb(gint x, gint y, GdkEvent *ev);

  void set_status(const std::string& s);

  void set_sex(ICQ2000::Sex s);
  void set_agerange(ICQ2000::AgeRange age);

  // -- library callbacks    --
  void result_cb(ICQ2000::SearchResultEvent *ev);
  void self_status_change_cb(ICQ2000::StatusChangeEvent *ev);

 public:
  SearchDialog(Gtk::Window * parent);

};

#endif
