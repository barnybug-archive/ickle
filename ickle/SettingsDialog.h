/* $Id: SettingsDialog.h,v 1.62 2004-07-03 16:40:25 cborni Exp $
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
#include <gtkmm/liststore.h>
#include <gtkmm/textview.h>
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
  bool m_icons_changed;
  bool m_fonts_changed;

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

  // look'n'feel
  Gtk::Button m_message_header_font;
  Gtk::Button m_message_text_font;

  // look'n'feel - charset
  Gtk::Entry m_lnf_charset;
  Gtk::Label m_lnf_charset_valid;

  // look'n'feel - contact list
  Gtk::CheckButton m_mouse_check_away_click;
  Gtk::CheckButton m_mouse_single_click;

  // look 'n''feel - icons
  std::string m_icons_dir;

  struct IconsModelColumns : public Gtk::TreeModel::ColumnRecord
  {
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  icon_online;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  icon_away;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  icon_na;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  icon_occupied;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  icon_dnd;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  icon_ffc;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  icon_offline;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  icon_invisible;
    Gtk::TreeModelColumn<Glib::ustring>                name;

    IconsModelColumns()
    {
      add(icon_online);
      add(icon_away);
      add(icon_na);
      add(icon_occupied);
      add(icon_dnd);
      add(icon_ffc);
      add(icon_offline);
      add(icon_invisible);
      add(name);
    }
  };

  const IconsModelColumns m_icons_columns;

  Glib::RefPtr<Gtk::TreeModel> m_icons_reftreemodel;
  Gtk::TreeView m_icons_treeview;

  // message window
  Gtk::CheckButton m_message_autoclose;
  Gtk::CheckButton m_message_autopopup;
  Gtk::CheckButton m_message_autoraise;

  Gtk::SpinButton m_history_shownr;
  
  Gtk::CheckButton m_spell_check;

  // Away/Idle page
  Gtk::SpinButton m_auto_away;
  Gtk::SpinButton m_auto_na;
  Gtk::CheckButton m_auto_return;

  // Away messages page
  struct AwayModelColumns : public Gtk::TreeModel::ColumnRecord
  {
    Gtk::TreeModelColumn<Glib::ustring> label;
    Gtk::TreeModelColumn<Glib::ustring> text;

    AwayModelColumns() { add(label); add(text); }
  };

  const AwayModelColumns m_away_columns;

  Glib::RefPtr<Gtk::ListStore> m_away_refliststore;
  Gtk::TreeView m_away_treeview;

  Gtk::TextView m_away_textview;
 
  Gtk::Button m_away_up_button;
  Gtk::Button m_away_remove_button;
  Gtk::Button m_away_down_button;
  Gtk::Button m_away_new_button;

  // Charset page
  Gtk::TreeModel::iterator m_row_lnf_charset;

  // Advanced Page
  Gtk::Entry m_network_login_host;
  Gtk::SpinButton m_network_login_port;
  Gtk::CheckButton m_network_override_port;

  // Advanced Page SMTP
  Gtk::CheckButton m_network_smtp;
  Gtk::Entry m_network_smtp_host;
  Gtk::SpinButton m_network_smtp_port;

  // Advanced Page Security
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

  // Events Page
  Gtk::Entry m_event_message;
  Gtk::Entry m_event_sms;
  Gtk::Entry m_event_system;
  Gtk::Entry m_event_url;
  Gtk::Entry m_event_user_online;
  Gtk::SpinButton m_event_repetition_threshold;
  Gtk::CheckButton m_event_execute_all;
  Gtk::Button m_substitutions;


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
  void set_message_header_font_cb();
  void set_message_text_font_cb();

  void choose_autoconnect (unsigned int s);

  void subs_cb();

  // Away messages
  void away_message_up_cb();
  void away_message_remove_cb();
  void away_message_down_cb();
  void away_message_new_cb();
  void away_message_select_cb();
  void away_message_text_edit_cb();

  // Icons
  void icons_changed();

  // Charset
  void lnf_charset_validate_cb();

 public:
  SettingsDialog(Gtk::Window& parent, bool start_on_away);
  ~SettingsDialog();

  SigC::Signal0<void> change_client; //signal that determines that new settings for the client are available
  SigC::Signal0<void> change_fonts; //signal that determines that new font settings are available
  SigC::Signal0<void> change_contact_list; //signal that determines that new settings for the contact list are available

  // TODO
};

#endif
