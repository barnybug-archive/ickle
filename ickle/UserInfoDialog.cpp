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

#include "UserInfoDialog.h"

#include "main.h"

#include "ickle.h"
#include "ucompose.h"
#include "utils.h"
#include "UserInfoHelpers.h"

#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>
#include <gtkmm/table.h>
#include <gtkmm/scrolledwindow.h>

#include <libicq2000/Client.h>

#include <vector>

using std::string;
using std::vector;
using std::map;

using namespace ICQ2000;

UserInfoDialog::UserInfoDialog(Gtk::Window& parent, const ContactRef& c, bool self)
  : Gtk::Dialog(),
    m_self(self),
    birth_year_spin((gfloat)1, 0), birth_month_spin((gfloat)1, 0),
    birth_day_spin((gfloat)1, 0),
    m_contact(c), m_changed(false)
{
  UserInfoHelpers::initialize();
  
  set_position(Gtk::WIN_POS_CENTER);
  set_transient_for(parent);

  if (m_self)
  {
    set_title( _("My User Info") );
  }
  else
  {
    if (c->isICQContact())
    {
      set_title( String::ucompose( _("User Info - %1 (%2)"),
				   Glib::ustring(c->getAlias()),
				   c->getUIN() ) );
    }
    else
    {
      set_title( String::ucompose( _("User Info - %1 (%2)"),
				   Glib::ustring(c->getAlias()),
				   Glib::ustring(c->getMobileNo()) ) );
    }
  }

  if (self)
    uploadb = add_button( _("_Upload"), RESPONSE_UPLOAD);
  
  fetchb = add_button(_("_Fetch"), RESPONSE_FETCH);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
  set_default_response(Gtk::RESPONSE_OK);

  notebook.set_tab_pos(Gtk::POS_TOP);

  Gtk::Label *label;

  // ******************** General Information ********************
  Gtk::Table *table = manage( new Gtk::Table( 4, 11, false ) );

  label = manage( new Gtk::Label( _("Alias:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND,Gtk::FILL | Gtk::EXPAND, 10);
  table->attach( alias_entry, 1, 4, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("UIN:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  uin_entry.set_editable(false);
  table->attach( uin_entry, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("IP:"), 0.0, 0.5 ) );
  table->attach( *label, 2, 3, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  ip_entry.set_editable(false);
  
  table->attach( ip_entry, 3, 4, 1, 2, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("Status:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  status_entry.set_editable(false);
  table->attach( status_entry, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("Timezone:"), 0.0, 0.5 ) );
  table->attach( *label, 2, 3, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  if (m_self) {
    timezone_combo.set_popdown_strings( UserInfoHelpers::getTimezoneAllStrings() );
    table->attach( timezone_combo, 3, 4, 2, 3, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);
  } else {
    timezone_entry.set_editable(false);
    table->attach( timezone_entry, 3, 4, 2, 3, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);
  }

  label = manage( new Gtk::Label( _("Name:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  table->attach( firstname_entry, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);
  table->attach( lastname_entry, 2, 4, 3, 4, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("Email 1:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 4, 5, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  table->attach( email_entry1, 1, 4, 4, 5, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);
  label = manage( new Gtk::Label( _("Email 2:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 5, 6, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  table->attach( email_entry2, 1, 4, 5, 6, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);
  label = manage( new Gtk::Label( _("Email 3:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 6, 7, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  table->attach( email_entry3, 1, 4, 6, 7, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);
  
  label = manage( new Gtk::Label( _("Address:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 7, 8, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  if (!m_self) addr_entry.set_editable(false);
  table->attach( addr_entry, 1, 2, 7, 8, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("Phone:"), 0.0, 0.5 ) );
  table->attach( *label, 2, 3, 7, 8, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  table->attach( phone_entry, 3, 4, 7, 8, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("State:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 8, 9, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  if (!m_self) state_entry.set_editable(false);
  table->attach( state_entry, 1, 2, 8, 9, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("Fax:"), 0.0, 0.5 ) );
  table->attach( *label, 2, 3, 8, 9, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  table->attach( fax_entry, 3, 4, 8, 9, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("City:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 9, 10, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  if (!m_self) city_entry.set_editable(false);
  table->attach( city_entry, 1, 2, 9, 10, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);
  
  label = manage( new Gtk::Label( _("Cellular:"), 0.0, 0.5 ) );
  table->attach( *label, 2, 3, 9, 10, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  table->attach( cellular_entry, 3, 4, 9, 10, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("Zip:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 10, 11, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  if (!m_self) zip_entry.set_editable(false);
  table->attach( zip_entry, 1, 2, 10, 11, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  label = manage( new Gtk::Label( _("Country:"), 0.0, 0.5 ) );
  table->attach( *label, 2, 3, 10, 11, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 10);
  if (m_self) {
    country_combo.set_popdown_strings( UserInfoHelpers::getCountryAllStrings() );
    table->attach( country_combo, 3, 4, 10, 11, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);
  } else {
    country_entry.set_editable(false);
    table->attach( country_entry, 3, 4, 10, 11, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);
  }

  table->set_border_width(5);
  table->set_row_spacings(5);
  table->set_col_spacing(1, 5);
  label = manage( new Gtk::Label( _("General") ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  // -------------------------- More Info Dialog -------------------
  table = manage( new Gtk::Table( 4, 11, false ) );

  label = manage( new Gtk::Label( _("Age:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 10);
  if (m_self) {
    Gtk::Adjustment *adj = manage( new Gtk::Adjustment( (gfloat)0, (gfloat)0, (gfloat)200) );
    age_spin.set_adjustment(*adj);
    age_spin.set_size_request(90, -1);
    table->attach( age_spin, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
    age_spin.signal_value_changed().connect( bind( slot( *this, &UserInfoDialog::spin_changed_cb ), &age_spin ) );
    spin_changed_cb(&age_spin);
  } else {
    age_entry.set_editable(false);
    table->attach( age_entry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  }
  
  label = manage( new Gtk::Label( _("Gender:"), 0.0, 0.5 ) );
  table->attach( *label, 2, 3, 0, 1, Gtk::FILL | Gtk::EXPAND,Gtk::FILL, 10);
  if (m_self) {
    table->attach( sex_combo, 3, 4, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  } else {
    sex_entry.set_editable(false);
    table->attach( sex_entry, 3, 4, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  }

  label = manage( new Gtk::Label( _("Homepage:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND,Gtk::FILL, 10);
  if (!m_self) homepage_entry.set_editable(false);
  table->attach( homepage_entry, 1, 4, 1, 2, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);

  label = manage( new Gtk::Label( _("Birthday:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 2, 3, Gtk::FILL | Gtk::EXPAND,Gtk::FILL, 10);
  if (m_self) {
    /* Add spinbuttons instead of entry */
    Gtk::HBox *hbox = manage( new Gtk::HBox() );
    label = manage( new Gtk::Label( _("Year: "), 0.0, 0.5 ) );
    hbox->pack_start( *label );
    Gtk::Adjustment *adj;
    adj = manage( new Gtk::Adjustment( (gfloat)1900, (gfloat)0, (gfloat)10000) );
    birth_year_spin.set_adjustment(*adj);
    hbox->pack_start( birth_year_spin );
    label = manage( new Gtk::Label( _("   Month: "), 0.0, 0.5 ) );
    hbox->pack_start( *label );
    adj = manage( new Gtk::Adjustment( (gfloat)1, (gfloat)1, (gfloat)12) );
    birth_month_spin.set_adjustment(*adj);
    hbox->pack_start( birth_month_spin );
    label = manage( new Gtk::Label( _("   Day: "), 0.0, 0.5 ) );
    hbox->pack_start( *label );
    adj = manage( new Gtk::Adjustment( (gfloat)1, (gfloat)1, (gfloat)31) );
    birth_day_spin.set_adjustment(*adj);
    hbox->pack_start( birth_day_spin );
    table->attach( *hbox, 1, 4, 2, 3, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  } else {
    birthday_entry.set_editable(false);
    table->attach( birthday_entry, 1, 4, 2, 3, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  }

  label = manage( new Gtk::Label( _("Language 1:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 3, 4, Gtk::FILL | Gtk::EXPAND,Gtk::FILL, 10);
  if (m_self) {
    lang_combo1.set_popdown_strings( UserInfoHelpers::getLanguageAllStrings() );
    table->attach( lang_combo1, 1, 4, 3, 4, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  } else {
    lang_entry1.set_editable(false);
    table->attach( lang_entry1, 1, 4, 3, 4, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  }
  label = manage( new Gtk::Label( _("Language 2:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 4, 5, Gtk::FILL | Gtk::EXPAND,Gtk::FILL, 10);
  if (m_self) {
    lang_combo2.set_popdown_strings( UserInfoHelpers::getLanguageAllStrings() );
    table->attach( lang_combo2, 1, 4, 4, 5, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  } else {
    lang_entry2.set_editable(false);
    table->attach( lang_entry2, 1, 4, 4, 5, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  }
  label = manage( new Gtk::Label( _("Language 3:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 5, 6, Gtk::FILL | Gtk::EXPAND,Gtk::FILL, 10);
  if (m_self) {
    lang_combo3.set_popdown_strings( UserInfoHelpers::getLanguageAllStrings() );
    table->attach( lang_combo3, 1, 4, 5, 6, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  } else {
    lang_entry3.set_editable(false);
    table->attach( lang_entry3, 1, 4, 5, 6, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL, 0);
  }

  table->set_border_width(5);
  table->set_row_spacings(5);
  label = manage( new Gtk::Label( _("More") ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  // ------------------------- About Dialog ------------------------
  table = manage( new Gtk::Table( 4, 11, false ) );

  label = manage( new Gtk::Label( _("About:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND,Gtk::FILL | Gtk::EXPAND, 10);
  about_text.set_editable(m_self);
  about_text.set_cursor_visible(m_self);
  about_text.set_wrap_mode(Gtk::WRAP_WORD);

  Gtk::ScrolledWindow *sc = manage(new Gtk::ScrolledWindow());
  sc->add(about_text);
  sc->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  sc->set_size_request(0, -1); // workaround for textview horizontal resizing
  sc->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
  table->attach( *sc, 0, 4, 1, 11, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK,Gtk::FILL | Gtk::EXPAND, 0);

  table->set_border_width(5);
  label = manage( new Gtk::Label( _("About") ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  // ------------------------- Stats -------------------------------
  table = manage( new Gtk::Table( 2, 5, false ) );
  
  label = manage( new Gtk::Label( _("Signon time:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 10);
  table->attach( stats_signon_time, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 0);

  label = manage( new Gtk::Label( _("Last time seen online:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 10);
  table->attach( stats_last_online, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 0);

  label = manage( new Gtk::Label( _("Last status change:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 10);
  table->attach( stats_last_status_change, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 0);

  label = manage( new Gtk::Label( _("Last message:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 10);
  table->attach( stats_last_message, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 0);

  label = manage( new Gtk::Label( _("Last checked your away message:"), 0.0, 0.5 ) );
  table->attach( *label, 0, 1, 4, 5, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 10);
  table->attach( stats_last_away_msg_check, 1, 2, 4, 5, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 0);

  table->set_border_width(5);
  table->set_row_spacings(5);
  label = manage( new Gtk::Label( _("Stats") ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  // ---------------------------------------------------------------

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing (10);
  vbox->pack_start( notebook, true, true );

  m_contact->status_change_signal.connect( this, &UserInfoDialog::status_change_cb );
  m_contact->userinfo_change_signal.connect( this, &UserInfoDialog::userinfo_change_cb );
  
  update_from_userinfo(); // fill in values
  
  set_border_width(10);
  show_all();
}

UserInfoDialog::~UserInfoDialog()
{
  m_signal_closed.emit();
}

void UserInfoDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    m_changed = update_contact() || m_changed;
    delete this;
  }
  else if (response_id == Gtk::RESPONSE_CANCEL)
  {
    delete this;
  }
  else if (response_id == RESPONSE_FETCH)
  {
    signal_fetch().emit();
  }
  else if (response_id == RESPONSE_UPLOAD)
  {
    m_changed = update_contact() || m_changed;
    signal_upload().emit();
  }
}

SigC::Signal0<void>& UserInfoDialog::signal_fetch()
{
  return m_signal_fetch;
}

SigC::Signal0<void>& UserInfoDialog::signal_upload()
{
  return m_signal_upload;
}

SigC::Signal0<void>& UserInfoDialog::signal_closed()
{
  return m_signal_closed;
}

bool UserInfoDialog::isChanged() const
{
  return m_changed;
}

bool UserInfoDialog::update_contact() 
{
  bool ret = false;

  if (m_self)
  {
    // self only editable fields
    Contact::MainHomeInfo& mhi = m_contact->getMainHomeInfo();
    
    if (mhi.street != addr_entry.get_text())
    {
      ret = true;
      mhi.street = addr_entry.get_text();
    }
    if (mhi.state != state_entry.get_text())
    {
      ret = true;
      mhi.state = state_entry.get_text();
    }
    if (mhi.city != city_entry.get_text())
    {
      ret = true;
      mhi.city = city_entry.get_text();
    }
    if (mhi.zip != zip_entry.get_text())
    {
      ret = true;
      mhi.zip = zip_entry.get_text();
    }

    /* Have to do some work to get the country */
    ICQ2000::Country country = UserInfoHelpers::getCountryFromString( country_combo.get_entry()->get_text() );
    if (mhi.country != country)
    {
      ret = true;
      mhi.country = country;
    }

    ICQ2000::Timezone timezone = UserInfoHelpers::getTimezoneFromString( timezone_combo.get_entry()->get_text() );
    if (mhi.timezone != timezone)
    {
      ret = true;
      mhi.timezone = timezone;
    }

    Contact::HomepageInfo &hpi = m_contact->getHomepageInfo();

    /* Get age from spin */
    ICQ2000::AgeRange age = (ICQ2000::AgeRange)age_spin.get_value_as_int();
    if (hpi.age != age)
    {
      ret = true;
      hpi.age = age;
    }

    /* Get gender from combo */
    ICQ2000::Sex sex = UserInfoHelpers::getSexFromString( sex_combo.get_entry()->get_text() );
    if (hpi.sex != sex)
    {
      ret = true;
      hpi.sex = sex;
    }
    
    if (hpi.homepage != homepage_entry.get_text())
    {
      ret = true;
      hpi.homepage = homepage_entry.get_text();
    }

    if (hpi.birth_year != (unsigned short)birth_year_spin.get_value_as_int())
    {
      ret = true;
      hpi.birth_year = (unsigned short)birth_year_spin.get_value_as_int();
    }

    if (hpi.birth_month != (unsigned char)birth_month_spin.get_value_as_int())
    {
      ret = true;
      hpi.birth_month = (unsigned char)birth_month_spin.get_value_as_int();
    }

    if (hpi.birth_day != (unsigned char)birth_day_spin.get_value_as_int())
    {
      ret = true;
      hpi.birth_day = (unsigned char)birth_day_spin.get_value_as_int();
    }

    ICQ2000::Language lang;

    lang = UserInfoHelpers::getLanguageFromString( lang_combo1.get_entry()->get_text() );
    if (hpi.lang1 != lang)
    {
      ret = true;
      hpi.lang1 = lang;
    }

    lang = UserInfoHelpers::getLanguageFromString( lang_combo2.get_entry()->get_text() );
    if (hpi.lang2 != lang)
    {
      ret = true;
      hpi.lang2 = lang;
    }

    lang = UserInfoHelpers::getLanguageFromString( lang_combo3.get_entry()->get_text() );
    if (hpi.lang3 != lang)
    {
      ret = true;
      hpi.lang3 = lang;
    }

    if ( m_contact->getAboutInfo() != about_text.get_buffer()->get_text() )
    {
      m_contact->setAboutInfo( about_text.get_buffer()->get_text() );
      ret = true;
    }

  } // self info

  if (m_contact->getAlias() != alias_entry.get_text())
  {
    ret = true;
    m_contact->setAlias(alias_entry.get_text());
  }
  if (m_contact->getMobileNo() != cellular_entry.get_text())
  {
    ret = true;
    m_contact->setMobileNo(cellular_entry.get_text());
  }
  if (m_contact->getFirstName() != firstname_entry.get_text())
  {
    ret = true;
    m_contact->setFirstName(firstname_entry.get_text());
  }
  if (m_contact->getLastName() != lastname_entry.get_text())
  {
    ret = true;
    m_contact->setLastName(lastname_entry.get_text());
  }
  if (m_contact->getEmail() != email_entry1.get_text())
  {
    ret = true;
    m_contact->setEmail(email_entry1.get_text());
  }

  Contact::MainHomeInfo& mhi = m_contact->getMainHomeInfo();
  if (mhi.phone != phone_entry.get_text())
  {
    ret = true;
    mhi.phone = phone_entry.get_text();
  }
  if (mhi.fax != fax_entry.get_text())
  {
    ret = true;
    mhi.fax = fax_entry.get_text();
  }
  if (mhi.getMobileNo() != cellular_entry.get_text())
  {
    ret = true;
    mhi.setMobileNo(cellular_entry.get_text());
  }
  return ret;
}

void UserInfoDialog::userinfo_change_cb(UserInfoChangeEvent *)
{
  update_from_userinfo();
}

void UserInfoDialog::update_from_userinfo() 
{
  if (m_contact->isICQContact()) uin_entry.set_text( m_contact->getStringUIN() );

  status_entry.set_text( UserInfoHelpers::getStringFromStatus(m_contact->getStatus()) );
  alias_entry.set_text( m_contact->getAlias() );
  firstname_entry.set_text( m_contact->getFirstName() );
  lastname_entry.set_text( m_contact->getLastName() );
  email_entry1.set_text( m_contact->getEmail() );
  city_entry.set_text( m_contact->getMainHomeInfo().city );
  state_entry.set_text( m_contact->getMainHomeInfo().state );
  phone_entry.set_text( m_contact->getMainHomeInfo().phone );
  fax_entry.set_text( m_contact->getMainHomeInfo().fax );
  addr_entry.set_text( m_contact->getMainHomeInfo().street );
  cellular_entry.set_text( m_contact->getMobileNo() );
  zip_entry.set_text( m_contact->getMainHomeInfo().zip );

  /* Set language */
  if (m_self)
  {
    lang_combo1.get_entry()->set_text(
	UserInfoHelpers::getStringFromLanguage( m_contact->getHomepageInfo().getLanguage(1) ) );
    lang_combo2.get_entry()->set_text(
	UserInfoHelpers::getStringFromLanguage( m_contact->getHomepageInfo().getLanguage(2) ) );
    lang_combo3.get_entry()->set_text(
	UserInfoHelpers::getStringFromLanguage( m_contact->getHomepageInfo().getLanguage(3) ) );
  }
  else
  {
    lang_entry1.set_text(
	UserInfoHelpers::getStringFromLanguage( m_contact->getHomepageInfo().getLanguage(1) ) );
    lang_entry2.set_text(
	UserInfoHelpers::getStringFromLanguage( m_contact->getHomepageInfo().getLanguage(2) ) );
    lang_entry3.set_text(
	UserInfoHelpers::getStringFromLanguage( m_contact->getHomepageInfo().getLanguage(3) ) );
  }
  
  if (m_contact->getExtIP() == 0 && m_contact->getLanIP() == 0 )
  {
    ip_entry.set_text( _("Unknown") );
  }
  else
  {
    ip_entry.set_text( String::ucompose( _("%1 / %2"),
					format_IP_and_port( m_contact->getLanIP(),
							    m_contact->getLanPort() ),
					format_IP_and_port( m_contact->getExtIP(),
							    m_contact->getExtPort() ) ) );
  }
  
  /* Set the right country in the combo, if we displaying info about ourselves we want to 
     be able to change country. If we're displaying info for another user we do not want to 
     edit country */
  if (m_self) {
    country_combo.get_entry()->set_text(
	UserInfoHelpers::getStringFromCountry( m_contact->getMainHomeInfo().getCountry() ) );
  } else {
    country_entry.set_text(
	UserInfoHelpers::getStringFromCountry( m_contact->getMainHomeInfo().getCountry() ) );
  }

  ICQ2000::Timezone timezone = m_contact->getMainHomeInfo().timezone;
  if (m_self)
  {
    timezone_combo.get_entry()->set_text( UserInfoHelpers::getStringFromTimezone(timezone) );
  }
  else
  {
    timezone_entry.set_text( UserInfoHelpers::getStringFromTimezone(timezone) );
  }
  
  // About box
  about_text.get_buffer()->set_text( m_contact->getAboutInfo() );

  // More info dialog
  if (m_self)
  {
    age_spin.set_value( m_contact->getHomepageInfo().age );
  }
  else
  {
    if (m_contact->getHomepageInfo().age == 0)
    {
      age_entry.set_text( _("Unspecified") );
    }
    else
    {
      age_entry.set_text( String::ucompose( _("%1 years"), (unsigned short int) m_contact->getHomepageInfo().age ) );
    }
  }

  if (m_self) {
    vector<string> gender;
    switch( m_contact->getHomepageInfo().sex )
    {
    case SEX_FEMALE:
      gender.push_back(_("Female"));
      gender.push_back(_("Male"));
      gender.push_back(_("Unspecified"));
      break;
    case SEX_MALE:
      gender.push_back(_("Male"));
      gender.push_back(_("Female"));
      gender.push_back(_("Unspecified"));
      break;
    default:
      gender.push_back(_("Unspecified"));
      gender.push_back(_("Male"));
      gender.push_back(_("Female"));
    }
    sex_combo.set_popdown_strings( gender );
  }
  else
  {
    sex_entry.set_text(UserInfoHelpers::getStringFromSex(m_contact->getHomepageInfo().sex));
  }
  homepage_entry.set_text( m_contact->getHomepageInfo().homepage );

  if (m_self)
  {
    birth_year_spin.set_value( (gfloat)m_contact->getHomepageInfo().birth_year );
    birth_month_spin.set_value( (gfloat)m_contact->getHomepageInfo().birth_month );
    birth_day_spin.set_value( (gfloat)m_contact->getHomepageInfo().birth_day );
  }
  else
  {
    birthday_entry.set_text( format_date( m_contact->getHomepageInfo().birth_day,
					  m_contact->getHomepageInfo().birth_month,
					  m_contact->getHomepageInfo().birth_year ) );
  }

  // ----------------------- Stats ----------------------

  stats_signon_time.set_text(format_time( m_contact->get_signon_time() ));
  stats_last_online.set_text(format_time( m_contact->get_last_online_time() ));
  stats_last_status_change.set_text(format_time( m_contact->get_last_status_change_time() ));
  stats_last_message.set_text(format_time( m_contact->get_last_message_time() ));
  stats_last_away_msg_check.set_text(format_time( m_contact->get_last_away_msg_check_time() ));

  // ----------------------------------------------------
}

Glib::ustring UserInfoDialog::format_time(time_t t)
{
  if (t == 0)
    return Glib::ustring(_("Unknown"));
  
  return Utils::format_time(t);
}

Glib::ustring UserInfoDialog::format_IP_and_port(unsigned int ip, unsigned short port)
{
  if (ip == 0)
    return Glib::ustring(_("Unknown"));

  if (port == 0)
    return String::ucompose( _("%1:?"), Utils::format_IP(ip) );

  return String::ucompose( _("%1:%2"), Utils::format_IP(ip), port );
}

Glib::ustring UserInfoDialog::format_date(unsigned char day, unsigned char month, unsigned short year)
{
  if (day == 0 || year == 0)
    return Glib::ustring(_("Unspecified"));

  return Utils::format_date(day, month, year);
}

void UserInfoDialog::status_change_cb(StatusChangeEvent *ev)
{
  fetchb->set_sensitive( ev->getStatus() != ICQ2000::STATUS_OFFLINE );
  if (m_self)
    uploadb->set_sensitive( ev->getStatus() != ICQ2000::STATUS_OFFLINE );
}

void UserInfoDialog::spin_changed_cb(Gtk::SpinButton *spin)
{
  if (spin->get_text() == "0") {
    spin->set_text(_("Unspecified"));
  }
  spin->set_position(0);
}
