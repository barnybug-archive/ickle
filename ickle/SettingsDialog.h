/* $Id: SettingsDialog.h,v 1.54 2003-04-07 07:21:46 cborni Exp $
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

#include <libicq2000/constants.h>

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
  bool m_client_restart;

  // the Tree store
  Glib::RefPtr<Gtk::TreeStore> m_reftreestore;

  // the Tree view
  Gtk::TreeView m_page_tree;

  Gtk::Label m_page_title;
  Gtk::Frame m_main_frame;

  // lots of widgets storing settings
  Gtk::Button m_apply_button, m_ok_button;

  // login page
  Gtk::SpinButton m_login_uin;
  Gtk::Entry m_login_pass;
  ICQ2000::Status m_auto_connect;
  Gtk::CheckButton m_auto_reconnect;
  Gtk::SpinButton m_reconnect_retries;

  // look'n'feel - charset
  Gtk::Entry m_lnf_charset;
  Gtk::Label m_lnf_charset_valid;

  // look'n'feel - contact list
  Gtk::CheckButton m_mouse_check_away_click;
  Gtk::CheckButton m_mouse_single_click;

  //lookn'n''feel - icons
  std::string m_icons_dir;


  // message window
  Gtk::CheckButton m_message_autoclose;
  Gtk::CheckButton m_message_autopopup;
  Gtk::CheckButton m_message_autoraise;

  Gtk::SpinButton m_history_shownr;

  // Away/Idle page
  Gtk::SpinButton m_auto_away;
  Gtk::SpinButton m_auto_na;
  Gtk::CheckButton m_auto_return;

  Gtk::TreeModel::iterator m_row_lnf_charset;

  // Advanced Page
  Gtk::Entry m_network_login_host;
  Gtk::SpinButton m_network_login_port;
  Gtk::CheckButton m_network_override_port;

  // Advanced Page SMTP
  Gtk::CheckButton m_network_smtp;
  Gtk::Entry m_network_smtp_host;
  Gtk::SpinButton m_network_smtp_port;

  // Advanced Page Securtiy
  Gtk::CheckButton m_network_in_dc;
  Gtk::CheckButton m_network_out_dc;
  Gtk::SpinButton m_network_lower_bind_port;
  Gtk::SpinButton m_network_upper_bind_port;
  Gtk::CheckButton m_network_use_portrange;

  // Advanced Page log
  Gtk::CheckButton m_log_to_console;
  Gtk::CheckButton m_log_to_file;
  Gtk::CheckButton m_log_directpacket;
  Gtk::CheckButton m_log_error;
  Gtk::CheckButton m_log_info;
  Gtk::CheckButton m_log_packet;
  Gtk::CheckButton m_log_warn;
  Gtk::Entry m_logfile;


  // page init functions
  void init_pages();
  void init_login_page();
  void init_look_page();
  void init_look_message_page();
  void init_look_contact_list_page();
  void init_look_icons_page();
  void init_look_charset_page();
  void init_events_page();
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
  void load_look_charset_page();
  void load_events_page();
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
  void save_look_charset_page();
  void save_events_page();
  void save_away_page();
  void save_away_idle_page();
  void save_away_message_page();
  void save_advanced_page();
  void save_advanced_security_page();
  void save_advanced_smtp_page();
  void save_advanced_logging_page();

  // validate entries
  bool validate_pages();
  bool validate_look_charset_page();

  Gtk::TreeModel::iterator add_page(const Glib::ustring& title, Gtk::Widget * page, bool toplevel);

  // various callbacks
  void page_tree_select_cb();
  void changed_cb();
  void toggle_smtp();
  void toggle_reconnect();
  void toggle_logfile();
  void client_changed();
  void toggle_dc(unsigned int what);
  void activate_changes();

  void on_apply_clicked();
  void on_ok_clicked();

  void choose_autoconnect (unsigned int s);
  void choose_icons_dir (const std::string dir);


  void lnf_charset_validate_cb();

 public:
  SettingsDialog(Gtk::Window& parent, bool start_on_away);
  ~SettingsDialog();

  SigC::Signal0<void> change_client; //signal that determines that new settings for the client are available
  SigC::Signal0<void> change_contact_list; //signal that determines that new settings for the contact list are available

  // TODO
};

#endif
