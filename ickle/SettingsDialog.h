/* $Id: SettingsDialog.h,v 1.45 2003-02-06 19:01:16 barnabygray Exp $
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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/button.h>
#include <gtkmm/tooltips.h>

class SettingsDialog : public Gtk::Dialog
{
 private:

  // Tree model columns
  struct ModelColumns : public Gtk::TreeModelColumnRecord
  {
    Gtk::TreeModelColumn< Glib::ustring > label;
    Gtk::TreeModelColumn< Gtk::Widget * > widget;

    ModelColumns()
    {
      add(label);
      add(widget);
    }
  };

  const ModelColumns m_columns;
  Gtk::Tooltips m_tooltip;

  // the Tree store
  Glib::RefPtr<Gtk::TreeStore> m_reftreestore;
  
  // the Tree view
  Gtk::TreeView m_page_tree;

  Gtk::Label m_page_title;
  Gtk::Frame m_main_frame;

  // lots of widgets storing settings
  Gtk::Button * m_apply_button;

  // login page
  Gtk::SpinButton m_login_uin;
  Gtk::Entry m_login_pass;


  // Away/Idle page
  Gtk::SpinButton m_auto_away;
  Gtk::SpinButton m_auto_na;
  Gtk::CheckButton m_auto_return;

  // page init functions
  void init_pages();
  void init_login_page();
  void init_look_page();
  void init_look_message_page();
  void init_look_contact_list_page();
  void init_look_icons_page();
  void init_away_page();
  void init_away_idle_page();
  void init_away_message_page();
  void init_advanced_page();
  void init_advanced_security_page();
  void init_advanced_smtp_page();
  void init_advanced_logging_page();

  // load from settings functions
  void load_pages();
  void load_login_page();
  void load_look_page();
  void load_look_message_page();
  void load_look_contact_list_page();
  void load_look_icons_page();
  void load_away_page();
  void load_away_idle_page();
  void load_away_message_page();
  void load_advanced_page();
  void load_advanced_security_page();
  void load_advanced_smtp_page();
  void load_advanced_logging_page();

  // save to settings functions
  void save_pages();
  void save_login_page();
  void save_look_page();
  void save_look_message_page();
  void save_look_contact_list_page();
  void save_look_icons_page();
  void save_away_page();
  void save_away_idle_page();
  void save_away_message_page();
  void save_advanced_page();
  void save_advanced_security_page();
  void save_advanced_smtp_page();
  void save_advanced_logging_page();

  void add_page(const Glib::ustring& title, Gtk::Widget * page, bool toplevel);

  // various callbacks
  void page_tree_select_cb();
  void changed_cb();

  virtual void on_response(int response_id);
  
 public:
  SettingsDialog(Gtk::Window& parent, bool start_on_away);
  ~SettingsDialog();
    
  // TODO
};

#endif
