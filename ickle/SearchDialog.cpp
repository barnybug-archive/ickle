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
    m_only_online_check("Only Online Users", 0),
    m_language_list(1),
    m_country_list(1)
{
  Gtk::Label *label;
  Gtk::Table *table;
  Gtk::Button *button;
  Gtk::Frame *frame;
  Gtk::ScrolledWindow *scrolled_window;

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
  
  table = manage( new Gtk::Table( 4, 5, false ) );
  
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
  Gtk::Menu *m = manage( new Gtk::Menu() );
  {
    using namespace Gtk::Menu_Helpers;
    MenuList& ml = m->items();
    ml.push_back( MenuElem( "Unspecified", bind( slot( this, &SearchDialog::set_sex ), SEX_UNSPECIFIED ) ) );
    ml.push_back( MenuElem( "Female", bind( slot( this, &SearchDialog::set_sex ), SEX_FEMALE ) ) );
    ml.push_back( MenuElem( "Male", bind( slot( this, &SearchDialog::set_sex ), SEX_MALE ) ) );
  }
  m_sex_menu.set_menu(*m);
  table->attach( *label, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_sex_menu, 1, 2, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

  label = manage( new Gtk::Label( "Minimum Age", 0 ) );
  m_min_age_spin.set_digits(0);
  Gtk::Adjustment *adj = m_min_age_spin.get_adjustment();
  adj->set_lower(0);
  adj->set_upper(99);
  adj->set_step_increment(1);
  adj->set_value(0);
  table->attach( *label, 2, 3, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_min_age_spin, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

  m_min_age_spin.changed.connect( bind( slot( this, &SearchDialog::spin_changed_cb ), &m_min_age_spin ) );
  m_min_age_spin.changed.emit();

  label = manage( new Gtk::Label( "Maximum Age", 0 ) );
  m_max_age_spin.set_digits(0);
  adj = m_max_age_spin.get_adjustment();
  adj->set_lower(0);
  adj->set_upper(99);
  adj->set_step_increment(1);
  adj->set_value(0);
  table->attach( *label, 2, 3, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  table->attach( m_max_age_spin, 3, 4, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

  m_max_age_spin.changed.connect( bind( slot( this, &SearchDialog::spin_changed_cb ), &m_max_age_spin ) );
  m_max_age_spin.changed.emit();

  label = manage( new Gtk::Label( "Language", 0, 0 ) );
  {
    using namespace Gtk::CList_Helpers;
    RowList& il = m_language_list.rows();
    for (int a = 0; a < Language_table_size; a++) {
      vector<const char*> r;
      r.push_back( Language_table[a] );
      il.push_back(r);
    }
  }
  m_language_list.set_selection_mode(GTK_SELECTION_BROWSE);
  m_language_list.row(0).select();
  m_language_list.columns_autosize();
  
  table->attach( *label, 2, 3, 2, 4, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);

  scrolled_window = manage(new Gtk::ScrolledWindow());
  scrolled_window->set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  scrolled_window->add(m_language_list);
  table->attach( *scrolled_window, 3, 4, 2, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  table->attach( m_only_online_check, 2, 3, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);
  m_reset_button.clicked.connect( slot( this, &SearchDialog::reset_cb ) );
  table->attach( m_reset_button, 3, 4, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

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
  
  label = manage( new Gtk::Label( "Country", 0, 0 ) );
  {
    using namespace Gtk::CList_Helpers;
    RowList& il = m_country_list.rows();
    for (int a = 0; a < Country_table_size; a++) {
      vector<const char*> r;
      r.push_back(Country_table[a].name);
      il.push_back(r);
    }
  }
  m_country_list.set_selection_mode(GTK_SELECTION_BROWSE);
  m_country_list.row(0).select();
  m_country_list.columns_autosize();

  table->attach( *label, 2, 3, 0, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND);
  scrolled_window = manage(new Gtk::ScrolledWindow());
  scrolled_window->set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  scrolled_window->add(m_country_list);
  table->attach( *scrolled_window, 3, 4, 0, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND);

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
    {
      using namespace Gtk::CList_Helpers;

      SelectionList& sl = m_language_list.selection();
      if (!sl.empty()) language = sl.begin()->get_row_num();
	
      SelectionList& sl2 = m_country_list.selection();
      if (!sl2.empty()) country = sl2.begin()->get_row_num();
			 
    }

    m_ev = icqclient.searchForContacts
      (m_alias_entry.get_text(),
       m_firstname_entry.get_text(),
       m_lastname_entry.get_text(),
       m_email_entry.get_text(),
       m_min_age_spin.get_value_as_int(),
       m_max_age_spin.get_value_as_int(),
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

      Contact *c = m_ev->getLastContactAdded();
      if (c != NULL) {
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

	Contact *cc = new Contact(*c);
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
  Contact *c = static_cast<Contact*>(row.get_data());
  icqclient.addContact( *c );
}

void SearchDialog::clist_data_destroy_cb(gpointer data)
{
  delete (Contact*)data;
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
  m_min_age_spin.set_value(0);
  m_max_age_spin.set_value(0);
  m_sex_menu.set_history(0);
  m_sex_selected = SEX_UNSPECIFIED;
  m_language_list.row(0).select();
  m_country_list.row(0).select();
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

void SearchDialog::spin_changed_cb(Gtk::SpinButton *spin)
{
  // hehe, I quite like this sneaky trick, bwaahahaha
  if (spin->get_text() == "0") {
    spin->set_text("Unspecified");
  }
  spin->set_position(0);
}