/*
 * SearchDialog
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

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <gtk--/dialog.h>
#include <gtk--/notebook.h>
#include <gtk--/entry.h>
#include <gtk--/combo.h>
#include <gtk--/clist.h>
#include <gtk--/button.h>
#include <gtk--/statusbar.h>
#include <gtk--/checkbutton.h>
#include <gtk--/spinbutton.h>
#include <gtk--/optionmenu.h>
#include <gtk--/list.h>

#include <libicq2000/events.h>
#include <libicq2000/userinfoconstants.h>

class SearchDialog : public Gtk::Dialog {
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

  // -- uin search --
  Gtk::Entry m_uin_entry;

  Gtk::CList m_clist;

  Gtk::Button m_ok_button, m_search_button, m_stop_button, m_add_button, m_reset_button;
  Gtk::Statusbar m_status;
  guint m_status_context;

  bool m_in_progress;
  ICQ2000::SearchResultEvent *m_ev;
  
  static void clist_data_destroy_cb(gpointer data);

 protected:
  void ok_cb();
  void search_cb();
  void stop_cb();
  void add_cb();
  void reset_cb();
  void result_cb(ICQ2000::SearchResultEvent *ev);
  void self_event_cb(ICQ2000::SelfEvent *ev);

  void select_row_cb(gint x, gint y, GdkEvent *ev);
  void unselect_row_cb(gint x, gint y, GdkEvent *ev);

  void set_status(const string& s);

  void set_sex(ICQ2000::Sex s);
  void set_agerange(ICQ2000::AgeRange age);

 public:
  SearchDialog();
  
};

#endif
