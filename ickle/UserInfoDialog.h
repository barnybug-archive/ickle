/*
 * UserInfoDialog
 * Copyright (C) 2001-2003 Barnaby Gray <barnaby@beedesign.co.uk>.
 * Copyright (C) 2003 Nils Nordman <nino@nforced.com>.
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

#ifndef USERINFODIALOG_H
#define USERINFODIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/notebook.h>
#include <gtkmm/textview.h>
#include <gtkmm/combo.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/textview.h>
#include <gtkmm/button.h>

#include <sigc++/signal.h>

#include <libicq2000/Contact.h>
#include <libicq2000/events.h>
#include <libicq2000/userinfoconstants.h>

class UserInfoDialog : public Gtk::Dialog,
		       public sigslot::has_slots<>
{
 private:
  enum
  {
    RESPONSE_UPLOAD = 1,
    RESPONSE_FETCH = 2
  };
    
  SigC::Signal0<void> m_signal_closed;
  SigC::Signal0<void> m_signal_fetch;
  SigC::Signal0<void> m_signal_upload;

  Gtk::Entry uin_entry, alias_entry, firstname_entry, lastname_entry,
    email_entry1, email_entry2, email_entry3, ip_entry, status_entry, timezone_entry,
    addr_entry, phone_entry, state_entry, fax_entry, city_entry, cellular_entry,
    zip_entry, country_entry, age_entry, sex_entry, homepage_entry, birthday_entry,
    lang_entry1, lang_entry2, lang_entry3;

  Gtk::SpinButton birth_year_spin, birth_month_spin, birth_day_spin, age_spin;
  Gtk::Combo country_combo, lang_combo1, lang_combo2, lang_combo3, sex_combo;
  Gtk::Combo timezone_combo;
  Gtk::TextView about_text;
  Gtk::Notebook notebook;

  Gtk::Entry stats_signon_time, stats_last_online, stats_last_status_change;
  Gtk::Entry stats_last_message, stats_last_away_msg_check;

  Gtk::Button *fetchb, *uploadb;

  ICQ2000::ContactRef m_contact;
  bool m_changed;
  bool m_self;

  void on_response(int response_id);
  bool update_contact();
  void update_from_userinfo();

  static Glib::ustring format_time(time_t t);
  static Glib::ustring format_IP_and_port(unsigned int ip, unsigned short port);
  static Glib::ustring format_date(unsigned char day, unsigned char month, unsigned short year);

  // -- library callbacks --
  void status_change_cb(ICQ2000::StatusChangeEvent *ev);
  void userinfo_change_cb(ICQ2000::UserInfoChangeEvent *ev);

  void spin_changed_cb(Gtk::SpinButton* spin);

 public:
  UserInfoDialog(Gtk::Window& parent, const ICQ2000::ContactRef& c, bool self);
  ~UserInfoDialog();

  SigC::Signal0<void>& signal_fetch();
  SigC::Signal0<void>& signal_upload();
  SigC::Signal0<void>& signal_closed();

  bool isChanged() const;
};

#endif
