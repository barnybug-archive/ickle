/*
 * UserInfoDialog
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

#include "UserInfoDialog.h"

#include "sstream_fix.h"
#include "main.h"

#include <gtk--/buttonbox.h>

#include <libicq2000/Client.h>
#include <libicq2000/socket.h>
#include <libicq2000/userinfohelpers.h>

#include <vector>

using std::ostringstream;
using std::vector;
using SigC::slot;

using namespace ICQ2000;

UserInfoDialog::UserInfoDialog(Gtk::Window * parent, const ContactRef& c, bool self)
  : Gtk::Dialog(), m_self(self),
    okay("OK"), cancel("Cancel"), fetchb("Fetch"), uploadb("Upload"),
    m_contact(c), m_changed(false), birth_year_spin((gfloat)1, 0), 
    birth_month_spin((gfloat)1, 0), birth_day_spin((gfloat)1, 0)
{
  set_transient_for (*parent);

  ostringstream ostr;
  if (m_self) {
    ostr << "My User Info";
  } else {
    ostr << "User Info - " << c->getAlias() << " (";
    if (c->isICQContact()) {
      ostr << c->getUIN();
    } else {
      ostr << c->getMobileNo();
    }
    ostr << ")";
  }
  
  set_title(ostr.str());

  okay.clicked.connect(slot(this,&UserInfoDialog::okay_cb));
  cancel.clicked.connect( destroy.slot() );
  fetchb.clicked.connect( fetch.slot() );
  uploadb.clicked.connect( slot(this,&UserInfoDialog::upload_cb) );

  ICQ2000::Status st = icqclient.getStatus();
  fetchb.set_sensitive(st != ICQ2000::STATUS_OFFLINE);
  uploadb.set_sensitive(st != ICQ2000::STATUS_OFFLINE);

  notebook.set_tab_pos(GTK_POS_TOP);

  Gtk::Label *label;

  Gtk::HBox *hbox = get_action_area();
  hbox->set_border_width(0);
  Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  hbbox->pack_start(fetchb, true, true, 0);
  if (self)
    hbbox->pack_start(uploadb, true, true, 0);
  hbbox->pack_start(okay, true, true, 0);
  hbbox->pack_start(cancel, true, true, 0);
  hbox->pack_start( *hbbox );

  // ******************** General Information ********************
  Gtk::Table *table = manage( new Gtk::Table( 4, 11, false ) );

  label = manage( new Gtk::Label( "Alias:", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND, 10);
  table->attach( alias_entry, 1, 4, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "UIN:", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  uin_entry.set_editable(false);
  table->attach( uin_entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "IP:", 0 ) );
  table->attach( *label, 2, 3, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  ip_entry.set_editable(false);
  
  table->attach( ip_entry, 3, 4, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Status:", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  status_entry.set_editable(false);
  table->attach( status_entry, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Timezone:", 0 ) );
  table->attach( *label, 2, 3, 2, 3, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  if (m_self) {
    timezone_combo.set_popdown_strings( Gtk::SArray(ICQ2000::UserInfoHelpers::getTimezoneAllStrings()) );
    table->attach( timezone_combo, 3, 4, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  } else {
    timezone_entry.set_editable(false);
    table->attach( timezone_entry, 3, 4, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  }

  label = manage( new Gtk::Label( "Name:", 0 ) );
  table->attach( *label, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  table->attach( firstname_entry, 1, 2, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  table->attach( lastname_entry, 2, 4, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Email 1:", 0 ) );
  table->attach( *label, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  table->attach( email_entry1, 1, 4, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  label = manage( new Gtk::Label( "Email 2:", 0 ) );
  table->attach( *label, 0, 1, 5, 6, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  table->attach( email_entry2, 1, 4, 5, 6, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  label = manage( new Gtk::Label( "Email 3:", 0 ) );
  table->attach( *label, 0, 1, 6, 7, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  table->attach( email_entry3, 1, 4, 6, 7, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  
  label = manage( new Gtk::Label( "Address:", 0 ) );
  table->attach( *label, 0, 1, 7, 8, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  if (!m_self) addr_entry.set_editable(false);
  table->attach( addr_entry, 1, 2, 7, 8, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Phone:", 0 ) );
  table->attach( *label, 2, 3, 7, 8, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  table->attach( phone_entry, 3, 4, 7, 8, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "State:", 0 ) );
  table->attach( *label, 0, 1, 8, 9, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  if (!m_self) state_entry.set_editable(false);
  table->attach( state_entry, 1, 2, 8, 9, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Fax:", 0 ) );
  table->attach( *label, 2, 3, 8, 9, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  table->attach( fax_entry, 3, 4, 8, 9, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "City:", 0 ) );
  table->attach( *label, 0, 1, 9, 10, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  if (!m_self) city_entry.set_editable(false);
  table->attach( city_entry, 1, 2, 9, 10, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  
  label = manage( new Gtk::Label( "Cellular:", 0 ) );
  table->attach( *label, 2, 3, 9, 10, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  table->attach( cellular_entry, 3, 4, 9, 10, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Zip:", 0 ) );
  table->attach( *label, 0, 1, 10, 11, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  if (!m_self) zip_entry.set_editable(false);
  table->attach( zip_entry, 1, 2, 10, 11, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Country:", 0 ) );
  table->attach( *label, 2, 3, 10, 11, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10);
  if (m_self) {
    country_combo.set_popdown_strings( Gtk::SArray(ICQ2000::UserInfoHelpers::getCountryAllStrings()) );
    table->attach( country_combo, 3, 4, 10, 11, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  } else {
    country_entry.set_editable(false);
    table->attach( country_entry, 3, 4, 10, 11, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  }

  table->set_border_width(5);
  table->set_row_spacings(5);
  label = manage( new Gtk::Label( "General" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  // -------------------------- More Info Dialog -------------------
  table = manage( new Gtk::Table( 4, 11, false ) );

  label = manage( new Gtk::Label( "Age:", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 10);
  if (m_self) {
    Gtk::Adjustment *adj = manage( new Gtk::Adjustment( (gfloat)0, (gfloat)0, (gfloat)200) );
    age_spin.set_adjustment(adj);
    age_spin.set_usize(90,0);
    table->attach( age_spin, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
    age_spin.changed.connect( bind( slot( this, &UserInfoDialog::spin_changed_cb ), &age_spin ) );
    age_spin.changed.emit();
  } else {
    age_entry.set_editable(false);
    table->attach( age_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }
  
  label = manage( new Gtk::Label( "Gender:", 0 ) );
  table->attach( *label, 2, 3, 0, 1, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  if (m_self) {
    table->attach( sex_combo, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  } else {
    sex_entry.set_editable(false);
    table->attach( sex_entry, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }

  label = manage( new Gtk::Label( "Homepage:", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  if (!m_self) homepage_entry.set_editable(false);
  table->attach( homepage_entry, 1, 4, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);

  label = manage( new Gtk::Label( "Birthday:", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  if (m_self) {
    /* Add spinbuttons instead of entry */
    Gtk::HBox *hbox = manage( new Gtk::HBox() );
    label = manage( new Gtk::Label( "Year: ", 0 ) );
    hbox->pack_start( *label );
    Gtk::Adjustment *adj;
    adj = manage( new Gtk::Adjustment( (gfloat)1900, (gfloat)0, (gfloat)10000) );
    birth_year_spin.set_adjustment(adj);
    hbox->pack_start( birth_year_spin );
    label = manage( new Gtk::Label( "   Month: ", 0 ) );
    hbox->pack_start( *label );
    adj = manage( new Gtk::Adjustment( (gfloat)1, (gfloat)1, (gfloat)12) );
    birth_month_spin.set_adjustment(adj);
    hbox->pack_start( birth_month_spin );
    label = manage( new Gtk::Label( "   Day: ", 0 ) );
    hbox->pack_start( *label );
    adj = manage( new Gtk::Adjustment( (gfloat)1, (gfloat)1, (gfloat)31) );
    birth_day_spin.set_adjustment(adj);
    hbox->pack_start( birth_day_spin );
    table->attach( *hbox, 1, 4, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  } else {
    birthday_entry.set_editable(false);
    table->attach( birthday_entry, 1, 4, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }

  label = manage( new Gtk::Label( "Language 1:", 0 ) );
  table->attach( *label, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  if (m_self) {
    lang_combo1.set_popdown_strings( Gtk::SArray( ICQ2000::UserInfoHelpers::getLanguageAllStrings() ) );
    table->attach( lang_combo1, 1, 4, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  } else {
    lang_entry1.set_editable(false);
    table->attach( lang_entry1, 1, 4, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }
  label = manage( new Gtk::Label( "Language 2:", 0 ) );
  table->attach( *label, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  if (m_self) {
    lang_combo2.set_popdown_strings( Gtk::SArray( ICQ2000::UserInfoHelpers::getLanguageAllStrings() ) );
    table->attach( lang_combo2, 1, 4, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  } else {
    lang_entry2.set_editable(false);
    table->attach( lang_entry2, 1, 4, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }
  label = manage( new Gtk::Label( "Language 3:", 0 ) );
  table->attach( *label, 0, 1, 5, 6, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  if (m_self) {
    lang_combo3.set_popdown_strings( Gtk::SArray( ICQ2000::UserInfoHelpers::getLanguageAllStrings() ) );
    table->attach( lang_combo3, 1, 4, 5, 6, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  } else {
    lang_entry3.set_editable(false);
    table->attach( lang_entry3, 1, 4, 5, 6, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }

  table->set_border_width(5);
  table->set_row_spacings(5);
  label = manage( new Gtk::Label( "More" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  // ------------------------- About Dialog ------------------------
  table = manage( new Gtk::Table( 4, 11, false ) );

  label = manage( new Gtk::Label( "About:", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND, 10);
  about_text.set_editable(m_self);
  table->attach( about_text, 0, 4, 1, 11, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  table->set_border_width(5);
  label = manage( new Gtk::Label( "About" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing (10);
  vbox->pack_start( notebook, true, true );

  m_contact->status_change_signal.connect( slot(this, &UserInfoDialog::status_change_cb) );
  m_contact->userinfo_change_signal.connect( slot(this, &UserInfoDialog::userinfo_change_cb) );
  
  update_from_userinfo(); // fill in values
  
  set_border_width(10);
  show_all();
}

UserInfoDialog::~UserInfoDialog() { }

bool UserInfoDialog::isChanged() const {
  return m_changed;
}

bool UserInfoDialog::update_contact() 
{
  bool ret = false;

  if (m_self) {
    // self only editable fields
    Contact::MainHomeInfo& mhi = m_contact->getMainHomeInfo();
    
    if (mhi.street != addr_entry.get_text()) {
      ret = true;
      mhi.street = addr_entry.get_text();
    }
    if (mhi.state != state_entry.get_text()) {
      ret = true;
      mhi.state = state_entry.get_text();
    }
    if (mhi.city != city_entry.get_text()) {
      ret = true;
      mhi.city = city_entry.get_text();
    }
    if (mhi.zip != zip_entry.get_text()) {
      ret = true;
      mhi.zip = zip_entry.get_text();
    }
    /* Have to do some work to get the country */
    unsigned short country = ICQ2000::UserInfoHelpers::getCountryStringtoID( country_combo.get_entry()->get_text() );
    if (mhi.country != country) {
      ret = true;
      mhi.country = country;
    }

    signed char timezone = ICQ2000::UserInfoHelpers::getTimezoneStringtoID( timezone_combo.get_entry()->get_text() );
    if (mhi.timezone != timezone) {
      ret = true;
      mhi.timezone = timezone;
    }

    Contact::HomepageInfo &hpi = m_contact->getHomepageInfo();

    /* Get age from spin */
    unsigned char age = (unsigned char)age_spin.get_value_as_int();
    if (hpi.age != age) {
      ret = true;
      hpi.age = age;
    }

    /* Get gender from combo */
    unsigned char sex = ICQ2000::UserInfoHelpers::getSexStringtoID( sex_combo.get_entry()->get_text() );
    if (hpi.sex != sex) {
      ret = true;
      hpi.sex = sex;
    }
    
    if (hpi.homepage != homepage_entry.get_text()) {
      ret = true;
      hpi.homepage = homepage_entry.get_text();
    }
    if (hpi.birth_year != (unsigned short)birth_year_spin.get_value_as_int()) {
      ret = true;
      hpi.birth_year = (unsigned short)birth_year_spin.get_value_as_int();
    }
    if (hpi.birth_month != (unsigned char)birth_month_spin.get_value_as_int()) {
      ret = true;
      hpi.birth_month = (unsigned char)birth_month_spin.get_value_as_int();
    }
    if (hpi.birth_day != (unsigned char)birth_day_spin.get_value_as_int()) {
      ret = true;
      hpi.birth_day = (unsigned char)birth_day_spin.get_value_as_int();
    }

    unsigned char lang;
    lang = ICQ2000::UserInfoHelpers::getLanguageStringtoID( lang_combo1.get_entry()->get_text() );
    if (hpi.lang1 != lang) {
      ret = true;
      hpi.lang1 = lang;
    }
    lang = ICQ2000::UserInfoHelpers::getLanguageStringtoID( lang_combo2.get_entry()->get_text() );
    if (hpi.lang2 != lang) {
      ret = true;
      hpi.lang2 = lang;
    }
    lang = ICQ2000::UserInfoHelpers::getLanguageStringtoID( lang_combo3.get_entry()->get_text() );
    if (hpi.lang3 != lang) {
      ret = true;
      hpi.lang3 = lang;
    }

    if ( m_contact->getAboutInfo() != about_text.get_chars(0,-1) ) {
      m_contact->setAboutInfo( about_text.get_chars(0,-1) );
      ret = true;
    }

  } // self info

  if (m_contact->getAlias() != alias_entry.get_text()) {
    ret = true;
    m_contact->setAlias(alias_entry.get_text());
  }
  if (m_contact->getMobileNo() != cellular_entry.get_text()) {
    ret = true;
    m_contact->setMobileNo(cellular_entry.get_text());
  }
  if (m_contact->getFirstName() != firstname_entry.get_text()) {
    ret = true;
    m_contact->setFirstName(firstname_entry.get_text());
  }
  if (m_contact->getLastName() != lastname_entry.get_text()) {
    ret = true;
    m_contact->setLastName(lastname_entry.get_text());
  }
  if (m_contact->getEmail() != email_entry1.get_text()) {
    ret = true;
    m_contact->setEmail(email_entry1.get_text());
  }
  Contact::MainHomeInfo& mhi = m_contact->getMainHomeInfo();
  if (mhi.phone != phone_entry.get_text()) {
    ret = true;
    mhi.phone = phone_entry.get_text();
  }
  if (mhi.fax != fax_entry.get_text()) {
    ret = true;
    mhi.fax = fax_entry.get_text();
  }
  if (mhi.getMobileNo() != cellular_entry.get_text()) {
    ret = true;
    mhi.setMobileNo(cellular_entry.get_text());
  }
  return ret;
}


void UserInfoDialog::okay_cb() {
  // check if anything was touched
  m_changed = update_contact() || m_changed;
  destroy.emit();
}

void UserInfoDialog::upload_cb()
{
  m_changed = update_contact() || m_changed;
  upload.emit();
}

void UserInfoDialog::raise() const {
  get_window().show();
}

void UserInfoDialog::userinfo_change_cb(UserInfoChangeEvent *ev) {
  update_from_userinfo();
}

void UserInfoDialog::update_from_userinfo() 
{
  if (m_contact->isICQContact()) uin_entry.set_text( m_contact->getStringUIN() );

  status_entry.set_text( m_contact->getStatusStr() );
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
  if (m_self) {
    lang_combo1.get_entry()->set_text( m_contact->getHomepageInfo().getLanguage(1) );
    lang_combo2.get_entry()->set_text( m_contact->getHomepageInfo().getLanguage(2) );
    lang_combo3.get_entry()->set_text( m_contact->getHomepageInfo().getLanguage(3) );
  } else {
    lang_entry1.set_text( m_contact->getHomepageInfo().getLanguage(1) );
    lang_entry2.set_text( m_contact->getHomepageInfo().getLanguage(2) );
    lang_entry3.set_text( m_contact->getHomepageInfo().getLanguage(3) );
  }
  
  ostringstream ostr;
  if (m_contact->getLanIP() == 0 && m_contact->getExtIP() == 0) {
    ostr << "Unknown";
  } else {
    ostr << IPtoString( m_contact->getLanIP() )
	 << ":"
	 << m_contact->getLanPort()
	 << " / "
	 << IPtoString( m_contact->getExtIP() )
	 << ":"
	 << m_contact->getExtPort();
  }
  
  ip_entry.set_text( ostr.str() );

  /* Set the right country in the combo, if we displaying info about ourselves we want to 
     be able to change country. If we're displaying info for another user we do not want to 
     edit country */
  if (m_self) {
    country_combo.get_entry()->set_text( m_contact->getMainHomeInfo().getCountry() );
  } else {
    country_entry.set_text(m_contact->getMainHomeInfo().getCountry());
  }

  signed char timezone = m_contact->getMainHomeInfo().timezone;
  if (m_self) {
    timezone_combo.get_entry()->set_text( ICQ2000::UserInfoHelpers::getTimezoneIDtoString(timezone) );
  } else {
    timezone_entry.set_text( ICQ2000::UserInfoHelpers::getTimezoneIDtoString(timezone) );
  }
  
  // About box
  about_text.delete_text(0,-1);
  about_text.insert( m_contact->getAboutInfo() );

  // More info dialog
  if (m_self) {
    age_spin.set_value( m_contact->getHomepageInfo().age );
  } else {
    if (m_contact->getHomepageInfo().age == 0) {
      age_entry.set_text( "Unspecified" );
    } else {
      ostringstream ostr;
      ostr << (unsigned int)m_contact->getHomepageInfo().age;
      age_entry.set_text( ostr.str() );
    }
  }

  if (m_self) {
    vector<string> gender;
    switch( m_contact->getHomepageInfo().sex ) {
    case SEX_FEMALE:
      gender.push_back("Female");
      gender.push_back("Male");
      gender.push_back("Unspecified");
      break;
    case SEX_MALE:
      gender.push_back("Male");
      gender.push_back("Female");
      gender.push_back("Unspecified");
      break;
    default:
      gender.push_back("Unspecified");
      gender.push_back("Male");
      gender.push_back("Female");
    }
    sex_combo.set_popdown_strings( gender );
  } else {
    switch( m_contact->getHomepageInfo().sex ) {
    case SEX_FEMALE:
      sex_entry.set_text("Female");
      break;
    case SEX_MALE:
      sex_entry.set_text("Male");
      break;
    default:
      sex_entry.set_text("Unspecified");
    }
  }
  homepage_entry.set_text( m_contact->getHomepageInfo().homepage );

  if (m_self) {
    birth_year_spin.set_value( (gfloat)m_contact->getHomepageInfo().birth_year );
    birth_month_spin.set_value( (gfloat)m_contact->getHomepageInfo().birth_month );
    birth_day_spin.set_value( (gfloat)m_contact->getHomepageInfo().birth_day );
  } else {
    birthday_entry.set_text( m_contact->getHomepageInfo().getBirthDate() );
  }
}

void UserInfoDialog::status_change_cb(StatusChangeEvent *ev)
{
  fetchb.set_sensitive( ev->getStatus() != ICQ2000::STATUS_OFFLINE );
  uploadb.set_sensitive( ev->getStatus() != ICQ2000::STATUS_OFFLINE );
}

void UserInfoDialog::spin_changed_cb(Gtk::SpinButton *spin)
{
  if (spin->get_text() == "0") {
    spin->set_text("Unspecified");
  }
  spin->set_position(0);
}
