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

#include <libicq2000/socket.h>
#include <libicq2000/userinfoconstants.h>

#include <vector>

using std::ostringstream;
using SigC::slot;
using namespace ICQ2000;

UserInfoDialog::UserInfoDialog(Contact *c, bool self)
  : Gtk::Dialog(), m_self(self),
    okay("OK"), cancel("Cancel"), fetchb("Fetch"),
    contact(c), changed(false), birth_year_spin((gfloat)1, 0), 
    birth_month_spin((gfloat)1, 0), birth_day_spin((gfloat)1, 0)
{
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

  notebook.set_tab_pos(GTK_POS_TOP);

  Gtk::Label *label;

  Gtk::HBox *hbox = get_action_area();
  hbox->pack_start(fetchb, true, true, 0);
  if (self) {
    Gtk::Button *uploadb = new Gtk::Button("Upload");
    uploadb->clicked.connect( slot(this,&UserInfoDialog::upload_cb) );
    hbox->pack_start(*uploadb, true, true, 0);
  }
  hbox->pack_start(okay, true, true, 0);
  hbox->pack_start(cancel, true, true, 0);

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
  timezone_entry.set_editable(false);
  table->attach( timezone_entry, 3, 4, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

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
    table->attach( country_combo, 3, 4, 10, 11, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  } else {
    country_entry.set_editable(false);
    table->attach( country_entry, 3, 4, 10, 11, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);
  }

  table->set_border_width(10);
  label = manage( new Gtk::Label( "General" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  // -------------------------- More Info Dialog -------------------
  table = manage( new Gtk::Table( 4, 11, false ) );

  label = manage( new Gtk::Label( "Age:", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 10);
  age_entry.set_editable(false);
  table->attach( age_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);

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
    hbox->pack_start( birth_year_spin );
    label = manage( new Gtk::Label( "   Month: ", 0 ) );
    hbox->pack_start( *label );
    hbox->pack_start( birth_month_spin );
    label = manage( new Gtk::Label( "   Day: ", 0 ) );
    hbox->pack_start( *label );
    hbox->pack_start( birth_day_spin );
    table->attach( *hbox, 1, 4, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  } else {
    birthday_entry.set_editable(false);
    table->attach( birthday_entry, 1, 4, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }

  label = manage( new Gtk::Label( "Language 1:", 0 ) );
  table->attach( *label, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  if (m_self) {
    table->attach( lang_combo1, 1, 4, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  } else {
    lang_entry1.set_editable(false);
    table->attach( lang_entry1, 1, 4, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }
  label = manage( new Gtk::Label( "Language 2:", 0 ) );
  table->attach( *label, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  if (m_self) {
    table->attach( lang_combo2, 1, 4, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  } else {
    lang_entry2.set_editable(false);
    table->attach( lang_entry2, 1, 4, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }
  label = manage( new Gtk::Label( "Language 3:", 0 ) );
  table->attach( *label, 0, 1, 5, 6, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  if (m_self) {
    table->attach( lang_combo3, 1, 4, 5, 6, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  } else {
    lang_entry3.set_editable(false);
    table->attach( lang_entry3, 1, 4, 5, 6, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);
  }

  table->set_border_width(10);
  table->set_row_spacings(5);
  label = manage( new Gtk::Label( "More" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  // ------------------------- About Dialog ------------------------
  table = manage( new Gtk::Table( 4, 11, false ) );

  label = manage( new Gtk::Label( "About:", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND, 10);
  if (!m_self) about_text.set_editable(false);
  table->attach( about_text, 0, 4, 1, 11, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

  table->set_border_width(10);
  label = manage( new Gtk::Label( "About" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  Gtk::VBox *vbox = get_vbox();
  vbox->pack_start( notebook, true, true );

  userinfochange_cb(); // fill in values

  set_border_width(10);
  show_all();
}

UserInfoDialog::~UserInfoDialog() { }

bool UserInfoDialog::isChanged() const {
  return changed;
}

bool UserInfoDialog::update_contact() 
{
  bool ret = false;

  if (m_self) {
    // self only editable fields
    MainHomeInfo& mhi = contact->getMainHomeInfo();
    
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
    string country = country_combo.get_entry()->get_text();
    if (mhi.getCountry() != country) {
      ret = true;
      for (int i = 0; i < Country_table_size; i++) {
	if ( country == string(Country_table[i].name)) {
	  mhi.country = Country_table[i].code;
	}
      }
    }
    HomepageInfo &hpi = contact->getHomepageInfo();
    /* Get age from entry */
    unsigned char age = (unsigned char)atoi(age_entry.get_text().c_str());
    if (hpi.age != age) {
      ret = true;
      hpi.age = age;
    }
    /* Get gender from combo */
    unsigned char sex;
    if (sex_combo.get_entry()->get_text() == "Male") {
      sex = SEX_MALE;
    } else if (sex_combo.get_entry()->get_text() == "Female") {
      sex = SEX_FEMALE;
    } else {
      sex = SEX_UNSPECIFIED;
    }

    if (hpi.sex != sex) {
      ret = true;
      hpi.sex = sex;
    }
    
    if (hpi.homepage != homepage_entry.get_text()) {
      ret = true;
      hpi.homepage = homepage_entry.get_text();
    }
    if (hpi.birth_year != (unsigned char)birth_year_spin.get_value_as_int()) {
      ret = true;
      hpi.birth_year = (unsigned char)birth_year_spin.get_value_as_int();
    }
    if (hpi.birth_month != (unsigned char)birth_month_spin.get_value_as_int()) {
      ret = true;
      hpi.birth_month = (unsigned char)birth_month_spin.get_value_as_int();
    }
    if (hpi.birth_day != (unsigned char)birth_day_spin.get_value_as_int()) {
      ret = true;
      hpi.birth_day = (unsigned char)birth_day_spin.get_value_as_int();
    }
    string lang1 = lang_combo1.get_entry()->get_text();
    string lang2 = lang_combo2.get_entry()->get_text();
    string lang3 = lang_combo3.get_entry()->get_text();

    for (int i = 0; i < Language_table_size; i++) {
      if (lang1 == string(Language_table[i])) {
	if (hpi.lang1 != i) {
	  ret = true;
	  hpi.lang1 = (unsigned char)i;
	}
      }
      if (lang2 == string(Language_table[i])) {
	if (hpi.lang2 != i) {
	  ret = true;
	  hpi.lang2 = (unsigned char)i;
	}
      }
      if (lang3 == string(Language_table[i])) {
	if (hpi.lang3 != i) {
	  ret = true;
	  hpi.lang3 = (unsigned char)i;
	}
      }
      
    }

    if ( contact->getAboutInfo() != about_text.get_chars() ) {
      contact->setAboutInfo( about_text.get_chars() );
      ret = true;
    }

  } // self info

  if (contact->getAlias() != alias_entry.get_text()) {
    ret = true;
    contact->setAlias(alias_entry.get_text());
  }
  if (contact->getMobileNo() != cellular_entry.get_text()) {
    ret = true;
    contact->setMobileNo(cellular_entry.get_text());
  }
  if (contact->getFirstName() != firstname_entry.get_text()) {
    ret = true;
    contact->setFirstName(firstname_entry.get_text());
  }
  if (contact->getLastName() != lastname_entry.get_text()) {
    ret = true;
    contact->setLastName(lastname_entry.get_text());
  }
  if (contact->getEmail() != email_entry1.get_text()) {
    ret = true;
    contact->setEmail(email_entry1.get_text());
  }
  MainHomeInfo& mhi = contact->getMainHomeInfo();
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
  changed = update_contact() || changed;
  destroy.emit();
}

void UserInfoDialog::upload_cb()
{
  changed = update_contact() || changed;
  upload.emit();
}

void UserInfoDialog::raise() const {
  get_window().show();
}

void UserInfoDialog::userinfochange_cb() {
  if (contact->isICQContact()) uin_entry.set_text( ICQ2000::Contact::UINtoString(contact->getUIN()) );

  status_entry.set_text( contact->getStatusStr() );
  alias_entry.set_text( contact->getAlias() );
  firstname_entry.set_text( contact->getFirstName() );
  lastname_entry.set_text( contact->getLastName() );
  email_entry1.set_text( contact->getEmail() );
  city_entry.set_text( contact->getMainHomeInfo().city );
  state_entry.set_text( contact->getMainHomeInfo().state );
  phone_entry.set_text( contact->getMainHomeInfo().phone );
  fax_entry.set_text( contact->getMainHomeInfo().fax );
  addr_entry.set_text( contact->getMainHomeInfo().street );
  cellular_entry.set_text( contact->getMobileNo() );
  zip_entry.set_text( contact->getMainHomeInfo().zip );

  /* Set language */
  if (m_self) {
    vector<string> languages1, languages2, languages3;
    languages1.push_back( contact->getHomepageInfo().getLanguage(1) );
    languages2.push_back( contact->getHomepageInfo().getLanguage(2) );
    languages3.push_back( contact->getHomepageInfo().getLanguage(3) );

    for (int i = 0; i < Language_table_size; i++) {
      languages1.push_back( string(Language_table[i]) );
      languages2.push_back( string(Language_table[i]) );
      languages3.push_back( string(Language_table[i]) );
    }
    lang_combo1.set_popdown_strings(languages1);
    lang_combo2.set_popdown_strings(languages2);
    lang_combo3.set_popdown_strings(languages3);
  } else {
    lang_entry1.set_text( contact->getHomepageInfo().getLanguage(1) );
    lang_entry2.set_text( contact->getHomepageInfo().getLanguage(2) );
    lang_entry3.set_text( contact->getHomepageInfo().getLanguage(3) );
  }
  
  ostringstream ostr;
  ostr << IPtoString( contact->getLanIP() )
       << ":"
       << contact->getLanPort()
       << " / "
       << IPtoString( contact->getExtIP() )
       << ":"
       << contact->getExtPort();

  ip_entry.set_text( ostr.str() );

  /* Set the right country in the combo, if we displaying info about ourselves we want to 
     be able to change country. If we're displaying info for another user we do not want to 
     edit country */
  if (m_self) {
    vector<string> countries;
    countries.push_back(contact->getMainHomeInfo().getCountry());
    for (int i = 0; i < Country_table_size; i++) {
      countries.push_back( string(Country_table[i].name) );
    }
    country_combo.set_popdown_strings(countries);
  } else {
    country_entry.set_text(contact->getMainHomeInfo().getCountry());
  }

  signed char timezone = contact->getMainHomeInfo().timezone;
  if (timezone == Timezone_unknown) {
    timezone_entry.set_text("Unknown");
  } else {
    ostringstream ostr;
    ostr << "GMT " << (timezone > 0 ? "-" : "+")
	 << abs(timezone/2)
	 << ":"
	 << (timezone % 2 == 0 ? "00" : "30");
    timezone_entry.set_text(ostr.str());
  }
  
  // About box
  about_text.delete_text(0,-1);
  about_text.insert( contact->getAboutInfo() );

  // More info dialog
  if (contact->getHomepageInfo().age == 0) {
    age_entry.set_text( "Unspecified" );
  } else {
    ostringstream ostr; //seekp doesn't work ;-/ clear() too. 
    ostr << (unsigned int)contact->getHomepageInfo().age;
    age_entry.set_text( ostr.str() );
  }

  if (m_self) {
    vector<string> gender;
    switch( contact->getHomepageInfo().sex ) {
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
    switch( contact->getHomepageInfo().sex ) {
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
  homepage_entry.set_text( contact->getHomepageInfo().homepage );

  if (m_self) {
    Gtk::Adjustment *adj;
    adj = manage( new Gtk::Adjustment( (gfloat)0, (gfloat)0, (gfloat)10000) );
    birth_year_spin.set_adjustment(adj);
    birth_year_spin.set_value( (gfloat)contact->getHomepageInfo().birth_year );
    adj = manage( new Gtk::Adjustment( (gfloat)0, (gfloat)1, (gfloat)12) );
    birth_month_spin.set_adjustment(adj);
    birth_month_spin.set_value( (gfloat)contact->getHomepageInfo().birth_month );
    adj = manage( new Gtk::Adjustment( (gfloat)0, (gfloat)0, (gfloat)31) );
    birth_day_spin.set_adjustment(adj);
    birth_day_spin.set_value( (gfloat)contact->getHomepageInfo().birth_day );
  } else {
    birthday_entry.set_text( contact->getHomepageInfo().getBirthDate() );
  }
}
