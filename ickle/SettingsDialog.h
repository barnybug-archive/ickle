/* $Id: SettingsDialog.h,v 1.40 2002-04-21 14:56:19 barnabygray Exp $
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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <gtk--/main.h>
#include <gtk--/tooltips.h>
#include <gtk--/dialog.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/entry.h>
#include <gtk--/notebook.h>
#include <gtk--/list.h>
#include <gtk--/checkbutton.h>
#include <gtk--/spinbutton.h>
#include <gtk--/radiobutton.h>
#include <gtk--/text.h>
#include <gtk--/optionmenu.h>
#include <gtk--/menu.h>
#include <gtk--/clist.h>
#include <gtk--/combo.h>
#include <string>
#include <vector>

#include "Settings.h"
#include "main.h"

#include <libicq2000/Contact.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

using std::string;

namespace Gtk {
  class FontSelectionDialog;
  class FileSelection;
}

class SettingsDialog : public Gtk::Dialog {
 private:
  Gtk::Tooltips m_tooltips;

  Gtk::Button okay, cancel, trans_b, subs_b;
  Gtk::Label trans_l;
  Gtk::Combo icons_combo;
  Gtk::CheckButton window_icons_check;
  Gtk::Entry uin_entry, password_entry;
  Gtk::Entry event_user_online_entry, event_message_entry, event_url_entry, event_sms_entry, event_system_entry;
  Gtk::SpinButton *event_repetition_spinner;
  Gtk::CheckButton event_execute_all;
  Gtk::Entry network_host;
  Gtk::Notebook notebook;
  Gtk::CheckButton away_autoposition, reconnect_checkbox, network_override_port;
  Gtk::CheckButton network_smtp;
  Gtk::Entry network_smtp_host;
  Gtk::SpinButton *network_smtp_port;
  Gtk::Label network_smtp_host_label, network_smtp_port_label;
  Gtk::CheckButton network_in_dc, network_out_dc;
  Gtk::CheckButton network_use_portrange;
  Gtk::SpinButton *network_lower_port_spinner, *network_upper_port_spinner;
  Gtk::Label network_lower_port_label, network_upper_port_label;
  Gtk::CheckButton message_autopopup, message_autoraise, message_autoclose;
  Gtk::RadioButton spell_check_no, spell_check_ispell, spell_check_aspell;
  Gtk::Entry spell_check_lang;
  Gtk::CheckButton mouse_single_click, mouse_check_away_click;
  Gtk::CheckButton status_cl_inv;
  Gtk::CheckButton popup_away_response;
  Gtk::SpinButton *reconnect_spinner, *network_port, *history_shownr_spinner;
  Gtk::SpinButton *autoaway_spinner, *autona_spinner;
  Gtk::Label reconnect_label, history_shownr_label;

  Gtk::CheckButton log_info, log_warn, log_error, log_packet, log_directpacket;
  Gtk::RadioButton log_to_nowhere, log_to_console, log_to_file, log_to_consolefile;

  /* Added widgets for away-msg editable */
  Gtk::Button away_remove_button;
  Gtk::Button away_up_button, away_down_button;
  Gtk::Text away_response_msg;
  Gtk::CList away_response_list;
  Gtk::Entry away_response_label_entry;
  std::vector<std::string> away_response_msg_list;
  std::vector<std::string> away_response_label_list;
  unsigned int away_current_item_number;
  bool away_response_label_edit_dead;
  
  string message_header_font, message_text_font;

  ICQ2000::Status m_status;

#ifdef GNOME_ICKLE
  Gtk::CheckButton *hidegui_onstart;
#endif

  bool finished_okay;

  string getIconsFilename();
  
  void setStatus(ICQ2000::Status s);

  void reconnect_toggle_cb();

  void okay_cb();
  void cancel_cb();
  void subs_cb();
  void trans_cb();
  void fontsel_cb(int n);
  void fontsel_ok_cb(Gtk::FontSelectionDialog *fontsel, int n);
  void icons_cb();
  void trans_ok_cb(Gtk::FileSelection *fs);

  void spinner_changed(Gtk::SpinButton *sb);

  void network_smtp_toggle_cb();
  void network_smtp_update();

  void network_port_range_toggle_cb();
  void network_port_range_update();
  void network_port_range_lower_cb();
  void network_port_range_upper_cb();

  void away_remove_button_cb();
  void away_up_button_cb();
  void away_down_button_cb();
  void away_response_list_select_row_cb(gint p0, gint p1, GdkEvent *ev);
  void away_response_select_row(unsigned int row);
  void away_response_label_edit();
  void away_response_buttons_update();
  
 public:
  SettingsDialog(Gtk::Window * parent);

  bool run();

  unsigned int getUIN() const;
  string getPassword() const;

  void updateSettings();
  void raise_away_status_tab();
  gint delete_event_impl(GdkEventAny*);
};

#endif
