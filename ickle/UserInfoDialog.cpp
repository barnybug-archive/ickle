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

using std::ostringstream;
using SigC::slot;
using namespace ICQ2000;

UserInfoDialog::UserInfoDialog(Contact *c, bool self)
  : Gtk::Dialog(), m_self(self),
    okay("OK"), cancel("Cancel"), fetchb("Fetch"),
    contact(c), changed(false)
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
  country_entry.set_editable(false);
  table->attach( country_entry, 3, 4, 10, 11, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL | GTK_EXPAND, 0);

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
  sex_entry.set_editable(false);
  table->attach( sex_entry, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);

  label = manage( new Gtk::Label( "Homepage:", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  homepage_entry.set_editable(false);
  table->attach( homepage_entry, 1, 4, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);

  label = manage( new Gtk::Label( "Birthday:", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  birthday_entry.set_editable(false);
  table->attach( birthday_entry, 1, 4, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);

  label = manage( new Gtk::Label( "Language 1:", 0 ) );
  table->attach( *label, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  lang_entry1.set_editable(false);
  table->attach( lang_entry1, 1, 4, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);

  label = manage( new Gtk::Label( "Language 2:", 0 ) );
  table->attach( *label, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  lang_entry2.set_editable(false);
  table->attach( lang_entry2, 1, 4, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);

  label = manage( new Gtk::Label( "Language 3:", 0 ) );
  table->attach( *label, 0, 1, 5, 6, GTK_FILL | GTK_EXPAND,GTK_FILL, 10);
  lang_entry3.set_editable(false);
  table->attach( lang_entry3, 1, 4, 5, 6, GTK_FILL | GTK_EXPAND | GTK_SHRINK,GTK_FILL, 0);

  table->set_border_width(10);
  table->set_row_spacings(5);
  label = manage( new Gtk::Label( "More" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem(*table, *label));

  // ------------------------- About Dialog ------------------------
  table = manage( new Gtk::Table( 4, 11, false ) );

  label = manage( new Gtk::Label( "About:", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND, 10);
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
    
    //    if (contact->
  }

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
  if (mhi.cellular != cellular_entry.get_text()) {
    ret = true;
    mhi.cellular = cellular_entry.get_text();
  }
  return ret;
}


void UserInfoDialog::okay_cb() {
  // check if anything was touched
  changed = changed || update_contact();
  destroy.emit();
}

void UserInfoDialog::upload_cb()
{
  changed = changed || update_contact();
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

  lang_entry1.set_text( contact->getHomepageInfo().getLanguage(1) );
  lang_entry2.set_text( contact->getHomepageInfo().getLanguage(2) );
  lang_entry3.set_text( contact->getHomepageInfo().getLanguage(3) );
  
  ostringstream ostr;
  ostr << IPtoString( contact->getLanIP() )
       << ":"
       << contact->getLanPort()
       << " / "
       << IPtoString( contact->getExtIP() )
       << ":"
       << contact->getExtPort();

  ip_entry.set_text( ostr.str() );

  country_entry.set_text( contact->getMainHomeInfo().getCountry() );
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

  homepage_entry.set_text( contact->getHomepageInfo().homepage );

  birthday_entry.set_text( contact->getHomepageInfo().getBirthDate() );

}
