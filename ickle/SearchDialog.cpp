/*
 * SearchDialog
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

#include "SearchDialog.h"

#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/table.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/buttonbox.h>
#include <gtk--/frame.h>
#include <gtk--/listitem.h>
#include <gtk--/menu.h>
#include <gtk--/menushell.h>
#include <gtk--/adjustment.h>

#include <libicq2000/Client.h>
#include <libicq2000/userinfoconstants.h>

#include "Icons.h"

#include "sstream_fix.h"

#include "main.h"

using SigC::slot;
using std::ostringstream;

using namespace ICQ2000;

SearchDialog::SearchDialog()
  : Gtk::Dialog(), m_clist(7), m_ev(NULL),
    m_ok_button("OK"), m_search_button("Search"), m_stop_button("Stop"),
    m_add_button("Add to List"), m_reset_button("Reset form"), m_sex_selected(SEX_UNSPECIFIED),
    m_agerange_selected(range_NoRange), m_only_online_check("Only Online Users", 0)
{
  Gtk::Label *label;
  Gtk::Table *table;
  Gtk::Button *button;
  Gtk::Frame *frame;
  Gtk::ScrolledWindow *scrolled_window;
  Gtk::Menu *m;
  
  set_title("Search for contacts");
  set_modal(false);
  m_notebook.set_tab_pos(GTK_POS_TOP);

  Gtk::HBox *hbox = get_action_area();
  
  Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  
  m_ok_button.clicked.connect( slot( this, &SearchDialog::ok_cb ) );
  hbbox->pack_start( m_ok_button );
  
  m_search_button.clicked.connect( slot( this, &SearchDialog::search_cb ) );
  hbbox->pack_start( m_search_button );

  m_stop_button.set_sensitive(false);
  m_stop_button.clicked.connect( slot( this, &SearchDialog::stop_cb ) );
  hbbox->pack_start( m_stop_button );

  m_add_button.set_sensitive(false);
  m_add_button.clicked.connect( slot( this, &SearchDialog::add_cb ) );
  hbbox->pack_start( m_add_button );

  hbox->pack_start( *hbbox );
  
  // ----- Whitepages -------

  Gtk::Table *ttable = manage( new Gtk::Table( 2, 2 ) );

  // -- Details --

  frame = manage( new Gtk::Frame("Details") );
  
  table = manage( new Gtk::Table( 4, 4, false ) );
  
  label = manage( new Gtk::Label( "Alias", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_alias_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);
  
  label = manage( new Gtk::Label( "First name", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_firstname_entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);
  
  label = manage( new Gtk::Label( "Last name", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_lastname_entry, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);
  
  label = manage( new Gtk::Label( "Email", 0 ) );
  table->attach( *label, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_email_entry, 1, 2, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);
  
  label = manage( new Gtk::Label( "Sex", 0 ) );
  m = manage( new Gtk::Menu() );
  {
    using namespace Gtk::Menu_Helpers;
    MenuList& ml = m->items();
    ml.push_back( MenuElem( "Unspecified", bind( slot( this, &SearchDialog::set_sex ), SEX_UNSPECIFIED ) ) );
    ml.push_back( MenuElem( "Female", bind( slot( this, &SearchDialog::set_sex ), SEX_FEMALE ) ) );
    ml.push_back( MenuElem( "Male", bind( slot( this, &SearchDialog::set_sex ), SEX_MALE ) ) );
  }
  m_sex_menu.set_menu(*m);
  table->attach( *label, 2, 3, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_sex_menu, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

  label = manage( new Gtk::Label( "Age Range", 0 ) );
  m = manage( new Gtk::Menu() );
  {
    using namespace Gtk::Menu_Helpers;
    MenuList& ml = m->items();
    ml.push_back( MenuElem( AgeRange_text[0], bind( slot( this, &SearchDialog::set_agerange ), range_NoRange ) ) );
    ml.push_back( MenuElem( AgeRange_text[1], bind( slot( this, &SearchDialog::set_agerange ), range_18_22 ) ) );
    ml.push_back( MenuElem( AgeRange_text[2], bind( slot( this, &SearchDialog::set_agerange ), range_23_29 ) ) );
    ml.push_back( MenuElem( AgeRange_text[3], bind( slot( this, &SearchDialog::set_agerange ), range_30_39 ) ) );
    ml.push_back( MenuElem( AgeRange_text[4], bind( slot( this, &SearchDialog::set_agerange ), range_40_49 ) ) );
    ml.push_back( MenuElem( AgeRange_text[5], bind( slot( this, &SearchDialog::set_agerange ), range_50_59 ) ) );
    ml.push_back( MenuElem( AgeRange_text[6], bind( slot( this, &SearchDialog::set_agerange ), range_60_above ) ) );
  }
  m_agerange_menu.set_menu(*m);
  table->attach( *label, 2, 3, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_agerange_menu, 3, 4, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

  label = manage( new Gtk::Label( "Language", 0) );

  vector<string> languages;
  for (int i = 0; i < Language_table_size; i++)
    languages.push_back( string(Language_table[i]) );
  m_language_combo.set_popdown_strings(languages);

  table->attach( *label, 2, 3, 2, 3, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_language_combo, 3, 4, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  table->attach( m_only_online_check, 2, 3, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);
  m_reset_button.clicked.connect( slot( this, &SearchDialog::reset_cb ) );
  table->attach( m_reset_button, 3, 4, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  table->set_spacings(3);
  table->set_col_spacing(1, 10);
  table->set_border_width(10);

  frame->add(*table);
  frame->set_border_width(5);

  ttable->attach( *frame, 0, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);

  // -- Location --

  frame = manage( new Gtk::Frame("Location") );
  
  table = manage( new Gtk::Table( 4, 2, false ) );
  
  label = manage( new Gtk::Label( "City", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  m_city_entry.set_usize(90,0);
  table->attach( m_city_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);
  
  label = manage( new Gtk::Label( "State", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  m_state_entry.set_usize(90,0);
  table->attach( m_state_entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);
  
  label = manage( new Gtk::Label( "Country", 0) );

  vector<string> countries;
  for (int i = 0; i < Country_table_size; i++)
    countries.push_back( string(Country_table[i].name) );
  m_country_combo.set_popdown_strings(countries);

  table->attach( *label, 2, 3, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_country_combo, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

  table->set_spacings(3);
  table->set_col_spacing(1, 10);
  table->set_border_width(10);

  frame->add(*table);
  frame->set_border_width(5);

  ttable->attach( *frame, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);

  // -- Work --

  frame = manage( new Gtk::Frame("Work") );
  
  table = manage( new Gtk::Table( 2, 3, false ) );
  
  label = manage( new Gtk::Label( "Position", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  m_position_entry.set_usize(90,0);
  table->attach( m_position_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);
  
  label = manage( new Gtk::Label( "Company Name", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  m_company_name_entry.set_usize(90,0);
  table->attach( m_company_name_entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);
  
  label = manage( new Gtk::Label( "Department", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  m_department_entry.set_usize(90,0);
  table->attach( m_department_entry, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

  table->set_spacings(3);
  table->set_border_width(10);

  frame->add(*table);
  frame->set_border_width(5);

  ttable->attach( *frame, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);

  /*
  */
  // -- end --

  label = manage( new Gtk::Label("Whitepages") );
  m_notebook.pages().push_back( Gtk::Notebook_Helpers::TabElem( *ttable, *label ) );

  // ------------------------

  // ----- UIN --------------

  table = manage( new Gtk::Table( 2, 1, false ) );
  
  label = manage( new Gtk::Label( "UIN", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_uin_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);
  
  table->set_border_width(10);
  label = manage( new Gtk::Label("UIN") );
  m_notebook.pages().push_back( Gtk::Notebook_Helpers::TabElem( *table, *label ) );

  Gtk::VBox *vbox = get_vbox();
  vbox->pack_start( m_notebook, false );

  m_clist.set_usize(0, 200);
  m_clist.set_column_title(0, "S");
  m_clist.set_column_title(1, "Alias");
  m_clist.set_column_min_width(1, 90);
  m_clist.set_column_title(2, "UIN");
  m_clist.set_column_min_width(2, 90);
  m_clist.set_column_title(3, "First name");
  m_clist.set_column_min_width(3, 90);
  m_clist.set_column_title(4, "Last name");
  m_clist.set_column_min_width(4, 90);
  m_clist.set_column_title(5, "Email");
  m_clist.set_column_min_width(5, 90);
  m_clist.set_column_title(6, "Auth. Req");
  m_clist.column_titles_show();
  m_clist.select_row.connect( slot( this, &SearchDialog::select_row_cb ) );
  m_clist.unselect_row.connect( slot( this, &SearchDialog::unselect_row_cb ) );

  Gtk::ScrolledWindow *scroll = manage( new Gtk::ScrolledWindow() );
  scroll->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  scroll->add(m_clist);
  vbox->pack_start( *scroll, true, true );

  m_status_context = m_status.get_context_id("searchdialog");
  vbox->pack_start( m_status, false );
  set_status( "Enter some information to search on above..." );
  
  show_all();

  // connect to Search Result signal on icqclient
  icqclient.search_result.connect( slot( this, &SearchDialog::result_cb ) );

  icqclient.self_contact_status_change_signal.connect( slot( this, &SearchDialog::self_status_change_cb ) );
}

void SearchDialog::ok_cb()
{
  destroy.emit();
}

void SearchDialog::search_cb()
{
  int n = m_notebook.get_current_page_num();
  if (n == 0) {

    unsigned char language = 0;
    unsigned short country = 0;

    string country_str = m_country_combo.get_entry()->get_text();
    for (int i = 0; i < Country_table_size; i++)
      if ( country_str == string(Country_table[i].name))
	country = i;

    string language_str = m_language_combo.get_entry()->get_text();
    for (unsigned char c = 0; c < Language_table_size; c++)
      if ( language_str == string(Language_table[c]))
	language = c;

    m_ev = icqclient.searchForContacts
      (m_alias_entry.get_text(),
       m_firstname_entry.get_text(),
       m_lastname_entry.get_text(),
       m_email_entry.get_text(),
       m_agerange_selected,
       m_sex_selected,
       language,
       m_city_entry.get_text(),
       m_state_entry.get_text(),
       Country_table[country].code,
       m_company_name_entry.get_text(),
       m_department_entry.get_text(),
       m_position_entry.get_text(),
       m_only_online_check.get_active());

  } else if (n == 1) {
    unsigned int uin = Contact::StringtoUIN( m_uin_entry.get_text() );
    if (uin == 0) return;
    m_ev = icqclient.searchForContacts(uin);
  }

  m_add_button.set_sensitive(false);
  m_search_button.set_sensitive(false);
  m_stop_button.set_sensitive(true);
  
  // clear any previous results
  m_clist.rows().clear();
  
  m_in_progress = true;
    
  set_status("Search in progress...");
}

void SearchDialog::result_cb(SearchResultEvent *ev)
{
  if (m_ev == ev) {
    // our search!!
    
    if (!m_ev->isExpired()) {

      ContactList& cl = m_ev->getContactList();

      ContactRef c = m_ev->getLastContactAdded();
      if (c.get() != NULL) {
	vector<string> row_array;
	row_array.push_back("");                // status icon
	row_array.push_back( c->getAlias() );
	row_array.push_back( c->getStringUIN() );
	row_array.push_back( c->getFirstName() );
	row_array.push_back( c->getLastName() );
	row_array.push_back( c->getEmail() );
	if (c->getAuthReq()) {
	  row_array.push_back( "Yes" );
	} else {
	  row_array.push_back( "No" );
	}
	  
	Gtk::CList_Helpers::RowIterator r = m_clist.rows().insert( m_clist.rows().end(), row_array );
	ImageLoader *p = g_icons.IconForStatus(c->getStatus(),false);
	(*r)[0].set_pixmap( p->pix(), p->bit() );

	/* the ContactRef needs to be dynamically allocated here
	   and deleted on clist destroy */
	ContactRef *cc = new ContactRef(c);
	(*r).set_data( cc, clist_data_destroy_cb );

	m_clist.columns_autosize();
      }
      
      ostringstream ostr;
      if (m_ev->isFinished()) {
	ostr << "Search finished, matches found: " << cl.size();
	if (m_ev->getNumberMoreResults() > 0) {
	  ostr << ", there are a further " << m_ev->getNumberMoreResults() << " matches not shown.";
	}
	
      } else {
	ostr << "Search in progress, matches found so far: " << cl.size();
      }
      set_status( ostr.str() );
    } else {
      set_status("Search timeout reached.");
    }
    
    if (m_ev->isFinished()) {
      m_in_progress = false;
      m_search_button.set_sensitive(true);
      m_stop_button.set_sensitive(false);
      m_ev = NULL;
    }
    
  }
  
}

void SearchDialog::self_status_change_cb(StatusChangeEvent *ev)
{
  m_search_button.set_sensitive( ev->getStatus() != ICQ2000::STATUS_OFFLINE );
}

void SearchDialog::stop_cb()
{
  m_in_progress = false;
  m_search_button.set_sensitive(true);
  m_stop_button.set_sensitive(false);
  m_ev = NULL;
  set_status("Search stopped.");
}

void SearchDialog::add_cb()
{
  using namespace Gtk::CList_Helpers;
  SelectionList& sl = m_clist.selection();
  if (sl.empty()) return;

  const Row& row = sl.front();
  ContactRef *c = static_cast<ContactRef*>(row.get_data());
  icqclient.addContact( *c );
}

void SearchDialog::clist_data_destroy_cb(gpointer data)
{
  delete (ContactRef*)data;
}

void SearchDialog::select_row_cb(gint x, gint y, GdkEvent *ev) 
{
  m_add_button.set_sensitive(true);
}

void SearchDialog::unselect_row_cb(gint x, gint y, GdkEvent *ev) 
{
  m_add_button.set_sensitive(false);
}

void SearchDialog::reset_cb()
{
  m_alias_entry.delete_text(0, -1);
  m_firstname_entry.delete_text(0, -1);
  m_lastname_entry.delete_text(0, -1);
  m_email_entry.delete_text(0, -1);
  m_city_entry.delete_text(0, -1);
  m_state_entry.delete_text(0, -1);
  m_company_name_entry.delete_text(0, -1);
  m_department_entry.delete_text(0, -1);
  m_position_entry.delete_text(0, -1);
  m_only_online_check.set_active(false);
  m_agerange_menu.set_history(0);
  m_agerange_selected = range_NoRange;
  m_sex_menu.set_history(0);
  m_sex_selected = SEX_UNSPECIFIED;
}

void SearchDialog::set_status( const string& text )
{
  if( m_status.messages().size() )
    m_status.pop( m_status_context );
  m_status.push( m_status_context, text);
}

void SearchDialog::set_sex( Sex s )
{
  m_sex_selected = s;
}

void SearchDialog::set_agerange( AgeRange r )
{
  m_agerange_selected = r;
}
