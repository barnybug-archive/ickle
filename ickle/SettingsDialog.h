/* $Id: SettingsDialog.h,v 1.30 2002-02-23 19:44:16 barnabygray Exp $
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
  Gtk::Button okay, cancel, trans_b, subs_b;
  Gtk::Label trans_l;
  Gtk::List icons_list;
  Gtk::Entry uin_entry, password_entry;
  Gtk::Entry event_user_online_entry, event_message_entry, event_url_entry, event_sms_entry;
  Gtk::Entry network_host;
  Gtk::Notebook notebook;
  Gtk::CheckButton away_autoposition, reconnect_checkbox, network_override_port;
  Gtk::CheckButton network_in_dc, network_out_dc;
  Gtk::CheckButton message_autopopup, message_autoraise, message_autoclose;
  Gtk::CheckButton spell_check, spell_check_aspell;
  Gtk::CheckButton mouse_single_click, mouse_check_away_click;
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

  void away_remove_button_cb();
  void away_up_button_cb();
  void away_down_button_cb();
  void away_response_list_select_row_cb(gint p0, gint p1, GdkEvent *ev);
  void away_response_select_row(unsigned int row);
  void away_response_label_edit();
  void away_response_buttons_update();
  
 public:
  SettingsDialog();

  bool run();

  unsigned int getUIN() const;
  string getPassword() const;

  void updateSettings();
  void raise_away_status_tab();
  gint delete_event_impl(GdkEventAny*);
};

#endif
