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

#include <gtkmm/stock.h>
#include <gtkmm/table.h>
#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>

#include "SearchDialog.h"
#include "AddContactDialog.h"

#include "ickle.h"

//#include "Icons.h"

#include "main.h"
#include "sstream_fix.h"
#include "UserInfoHelpers.h"

using std::string;
using std::vector;
using std::ostringstream;

using SigC::slot;

using ICQ2000::ContactRef;
using namespace ICQ2000;

SearchDialog::SearchDialog(Gtk::Window * parent)
  : Gtk::Dialog(), m_only_online_check(_("Only Online Users"), 0),
    m_sex_selected(SEX_UNKNOWN), m_agerange_selected(),
    m_treeview(), m_ok_button(Gtk::Stock::OK),
    m_search_button(Gtk::Stock::FIND),
    m_add_button(Gtk::Stock::ADD),
    m_stop_button(Gtk::Stock::STOP)
{
  Gtk::Label *label;
  Gtk::Table *table;
  Gtk::Frame *frame;
  Gtk::Menu *m;

  set_title( _("Search for contacts") );
  set_transient_for (*parent);
  set_modal(false);

  m_notebook.set_tab_pos(Gtk::POS_TOP);

  Gtk::HButtonBox *hbbox = get_action_area();
  //  hbox->set_border_width(0);
  
  //Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  
  m_ok_button.signal_clicked().connect( slot( *this, &SearchDialog::ok_cb ) );
  hbbox->pack_start( m_ok_button );
  
  m_search_button.signal_clicked().connect( slot( *this, &SearchDialog::search_cb ) );
  hbbox->pack_start( m_search_button );

  m_stop_button.set_sensitive(false);
  m_stop_button.signal_clicked().connect( slot( *this, &SearchDialog::stop_cb ) );
  hbbox->pack_start( m_stop_button );

  m_add_button.set_sensitive(false);
  m_add_button.signal_clicked().connect( slot( *this, &SearchDialog::add_cb ) );
  hbbox->pack_start( m_add_button );

  //  hbox->pack_start( *hbbox );
  
  // ----- Whitepages -------

  Gtk::Table *ttable = manage( new Gtk::Table( 2, 2 ) );

  // -- Details --

  frame = manage( new Gtk::Frame( _("Details") ) );
  
  table = manage( new Gtk::Table( 4, 4, false ) );
  
  label = manage( new Gtk::Label( _("Alias"), 0 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_alias_entry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  label = manage( new Gtk::Label( _("First name"), 0 ) );
  table->attach( *label, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_firstname_entry, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  label = manage( new Gtk::Label( _("Last name"), 0 ) );
  table->attach( *label, 0, 1, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_lastname_entry, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  label = manage( new Gtk::Label( _("Email"), 0 ) );
  table->attach( *label, 0, 1, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_email_entry, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  label = manage( new Gtk::Label( _("Sex"), 0 ) );
  m = manage( new Gtk::Menu() );
  {
    using namespace Gtk::Menu_Helpers;
    MenuList& ml = m->items();
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromSex(SEX_UNKNOWN),
			    bind( slot( *this, &SearchDialog::set_sex ), SEX_UNKNOWN ) ) );
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromSex(SEX_FEMALE),
			    bind( slot( *this, &SearchDialog::set_sex ), SEX_FEMALE ) ) );
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromSex(SEX_MALE),
			    bind( slot( *this, &SearchDialog::set_sex ), SEX_MALE ) ) );
  }
  m_sex_menu.set_menu(*m);
  table->attach( *label, 2, 3, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_sex_menu, 3, 4, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

  label = manage( new Gtk::Label( _("Age Range"), 0 ) );
  m = manage( new Gtk::Menu() );
  {
    using namespace Gtk::Menu_Helpers;
    MenuList& ml = m->items();
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromAgeRange(RANGE_NORANGE),
			    bind( slot( *this, &SearchDialog::set_agerange ), RANGE_NORANGE ) ) );
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromAgeRange(RANGE_18_22),
			    bind( slot( *this, &SearchDialog::set_agerange ), RANGE_18_22 ) ) );
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromAgeRange(RANGE_23_29),
			    bind( slot( *this, &SearchDialog::set_agerange ), RANGE_23_29 ) ) );
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromAgeRange(RANGE_30_39),
			    bind( slot( *this, &SearchDialog::set_agerange ), RANGE_30_39 ) ) );
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromAgeRange(RANGE_40_49),
			    bind( slot( *this, &SearchDialog::set_agerange ), RANGE_40_49 ) ) );
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromAgeRange(RANGE_50_59),
			    bind( slot( *this, &SearchDialog::set_agerange ), RANGE_50_59 ) ) );
    ml.push_back( MenuElem( UserInfoHelpers::getStringFromAgeRange(RANGE_60_ABOVE),
			    bind( slot( *this, &SearchDialog::set_agerange ), RANGE_60_ABOVE ) ) );
  }
  m_agerange_menu.set_menu(*m);
  table->attach( *label, 2, 3, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_agerange_menu, 3, 4, 1, 2, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

  label = manage( new Gtk::Label( "Language", 0) );

  m_language_combo.set_popdown_strings(UserInfoHelpers::getLanguageAllStrings());

  table->attach( *label, 2, 3, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_language_combo, 3, 4, 2, 3, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK);

  table->attach( m_only_online_check, 2, 3, 3, 4, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK);
  m_reset_button.set_label(_("Reset") );
  m_reset_button.signal_clicked().connect( slot( *this, &SearchDialog::reset_cb ) );
  table->attach( m_reset_button, 3, 4, 3, 4, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK);

  table->set_spacings(3);
  table->set_col_spacing(1, 10);
  table->set_border_width(5);

  frame->add(*table);
  frame->set_border_width(5);

  ttable->attach( *frame, 0, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);

  // -- Location --

  frame = manage( new Gtk::Frame( _("Location") ) );
  
  table = manage( new Gtk::Table( 4, 2, false ) );
  
  label = manage( new Gtk::Label( _("City"), 0 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  m_city_entry.set_size_request(90,0);
  table->attach( m_city_entry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  label = manage( new Gtk::Label( _("State"), 0 ) );
  table->attach( *label, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  m_state_entry.set_size_request(90,0);
  table->attach( m_state_entry, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  label = manage( new Gtk::Label( _("Country"), 0) );

  m_country_combo.set_popdown_strings( UserInfoHelpers::getCountryAllStrings() );

  table->attach( *label, 2, 3, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_country_combo, 3, 4, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

  table->set_spacings(3);
  table->set_col_spacing(1, 10);
  table->set_border_width(5);

  frame->add(*table);
  frame->set_border_width(5);

  ttable->attach( *frame, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);

  // -- Work --

  frame = manage( new Gtk::Frame( _("Work") ) );
  
  table = manage( new Gtk::Table( 2, 3, false ) );
  
  label = manage( new Gtk::Label( _("Position"), 0 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  m_position_entry.set_size_request(90,0);
  table->attach( m_position_entry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  label = manage( new Gtk::Label( _("Company Name"), 0 ) );
  table->attach( *label, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  m_company_name_entry.set_size_request(90,0);
  table->attach( m_company_name_entry, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  label = manage( new Gtk::Label( _("Department"), 0 ) );
  table->attach( *label, 0, 1, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  m_department_entry.set_size_request(90,0);
  table->attach( m_department_entry, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

  table->set_spacings(3);
  table->set_border_width(5);

  frame->add(*table);
  frame->set_border_width(5);

  ttable->attach( *frame, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);

  /*
  */
  // -- end --

  label = manage( new Gtk::Label( _("Whitepages") ) );
  m_notebook.pages().push_back( Gtk::Notebook_Helpers::TabElem( *ttable, *label ) );

  // ------------------------

  // ----- UIN --------------

  table = manage( new Gtk::Table( 2, 1, false ) );
  
  label = manage( new Gtk::Label( _("UIN"), 0 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_uin_entry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  table->set_border_width(5);
  label = manage( new Gtk::Label( _("UIN") ) );
  m_notebook.pages().push_back( Gtk::Notebook_Helpers::TabElem( *table, *label ) );

  // ------------------------

  // ----- Keyword Search -----------

  table = manage( new Gtk::Table( 2, 1, false ) );
  
  label = manage( new Gtk::Label( _("Keywords"), 0 ) );
  table->attach( *label, 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
  table->attach( m_keyword_entry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND | Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
  
  table->set_border_width(5);
  label = manage( new Gtk::Label( _("Keywords") ) );
  m_notebook.pages().push_back( Gtk::Notebook_Helpers::TabElem( *table, *label ) );

  // ------------------------

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  vbox->pack_start( m_notebook, false, false);

  m_ref_liststore = Gtk::ListStore::create(m_columns);
  m_treeview.set_model(m_ref_liststore);
  m_treeview.set_rules_hint();
  Glib::RefPtr<Gtk::TreeSelection> selection = m_treeview.get_selection();
  selection->signal_changed().connect(slot(*this, &SearchDialog::select_row_cb));
  int col_cnt;
  {
    Gtk::CellRendererText* pRenderer = Gtk::manage( new Gtk::CellRendererText() );

    col_cnt =m_treeview.append_column(_("Alias"), *pRenderer);
    Gtk::TreeViewColumn* pColumn = m_treeview.get_column(col_cnt - 1);

    pColumn->add_attribute(pRenderer->property_text(), m_columns.alias);
  }
  {
    Gtk::CellRendererText* pRenderer = Gtk::manage( new Gtk::CellRendererText() );

    col_cnt =m_treeview.append_column(_("UIN"), *pRenderer);
    Gtk::TreeViewColumn* pColumn = m_treeview.get_column(col_cnt - 1);

    pColumn->add_attribute(pRenderer->property_text(), m_columns.uin);
  }
  {
    Gtk::CellRendererText* pRenderer = Gtk::manage( new Gtk::CellRendererText() );

    col_cnt =m_treeview.append_column(_("First name"), *pRenderer);
    Gtk::TreeViewColumn* pColumn = m_treeview.get_column(col_cnt - 1);

    pColumn->add_attribute(pRenderer->property_text(), m_columns.first_name);
  }
  {
    Gtk::CellRendererText* pRenderer = Gtk::manage( new Gtk::CellRendererText() );

    col_cnt =m_treeview.append_column(_("Last name"), *pRenderer);
    Gtk::TreeViewColumn* pColumn = m_treeview.get_column(col_cnt - 1);

    pColumn->add_attribute(pRenderer->property_text(), m_columns.last_name);
  }
  {
    Gtk::CellRendererText* pRenderer = Gtk::manage( new Gtk::CellRendererText() );

    col_cnt =m_treeview.append_column(_("E-mail"), *pRenderer);
    Gtk::TreeViewColumn* pColumn = m_treeview.get_column(col_cnt - 1);

    pColumn->add_attribute(pRenderer->property_text(), m_columns.email);
  }
  {
    Gtk::CellRendererToggle* pRenderer = Gtk::manage( new Gtk::CellRendererToggle() );
    
    col_cnt =m_treeview.append_column(_("Auth. req."), *pRenderer);
    Gtk::TreeViewColumn* pColumn = m_treeview.get_column(col_cnt - 1);
    
    pColumn->add_attribute(pRenderer->property_active(), m_columns.auth_req);
  }

  Gtk::ScrolledWindow *scroll = manage( new Gtk::ScrolledWindow() );
  scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scroll->add(m_treeview);
  vbox->pack_start( *scroll, true, true );

  m_status_context = m_status.get_context_id( _("searchdialog") );
  vbox->pack_start( m_status, false, false );
  set_status( _("Enter some information to search on above...") );

  set_border_width(10);

// -- group frame
  frame = manage( new Gtk::Frame( _("Details") ) );
  table = manage( new Gtk::Table( 2, 1, false ) );
// group list
  ICQ2000::ContactTree& ct = icqclient.getContactTree();

  if ( ! ct.empty() )
  {
    ICQ2000::ContactTree::iterator curr = ct.begin();
    Gtk::ComboDropDown_Helpers::ComboDropDownList& cl = m_group_list.get_list()->children();
    while (curr != ct.end())
    {
      Gtk::ComboDropDownItem * item = Gtk::manage( new Gtk::ComboDropDownItem() );
      item->add_label( curr->get_label() );
      item->show_all();
      item->signal_select().connect( SigC::bind( SigC::slot( *this, &SearchDialog::selected_group_cb ), &(*curr) ) );
      cl.push_back( *item );
      m_group_list.set_item_string( *item, curr->get_label() );
      m_group_map[ curr->get_id() ] = item;

      ++curr;
    }
  }
  else
  {
    std::list< std::string > strlist;
    strlist.push_back( _("New") );
    m_selected_group = NULL;
    m_group_list.set_popdown_strings( strlist );
  }

  m_group_list.set_value_in_list();
  table->attach( * manage( new Gtk::Label( _("Add to group"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );

  table->attach( m_group_list, 1, 2, 0, 1, Gtk::AttachOptions(0), Gtk::FILL | Gtk::EXPAND );
  frame->add( *table );
  vbox->pack_start( *frame );


  show_all();

  // connect to Search Result signal on icqclient
  icqclient.search_result.connect( this, &SearchDialog::result_cb );

  icqclient.self_contact_status_change_signal.connect( this, &SearchDialog::self_status_change_cb );

  
  show_all();
}

void SearchDialog::clist_data_destroy_cb(gpointer data)
{

}

void SearchDialog::ok_cb()
{

}

void SearchDialog::search_cb()
{
  int n = m_notebook.get_current_page();
  if (n == 0) {

    Language language = UserInfoHelpers::getLanguageFromString( m_language_combo.get_entry()->get_text() );
    Country country   = UserInfoHelpers::getCountryFromString( m_country_combo.get_entry()->get_text() );

    m_ev = icqclient.searchForContacts
      (Glib::locale_from_utf8( m_alias_entry.get_text() ),
       Glib::locale_from_utf8( m_firstname_entry.get_text() ),
       Glib::locale_from_utf8( m_lastname_entry.get_text() ),
       m_email_entry.get_text(),
       m_agerange_selected,
       m_sex_selected,
       language,
       Glib::locale_from_utf8( m_city_entry.get_text() ),
       m_state_entry.get_text(),
       country,
       Glib::locale_from_utf8( m_company_name_entry.get_text() ),
       Glib::locale_from_utf8( m_department_entry.get_text() ),
       Glib::locale_from_utf8( m_position_entry.get_text() ),
       m_only_online_check.get_active());

  } else if (n == 1) {
    unsigned int uin = Contact::StringtoUIN( m_uin_entry.get_text() );
    if (uin == 0) return;
    m_ev = icqclient.searchForContacts(uin);

  } else if (n == 2) {
    string kw = m_keyword_entry.get_text();
    if (kw.empty()) return;
    m_ev = icqclient.searchForContacts(kw);
  }

  m_add_button.set_sensitive(false);
  m_search_button.set_sensitive(false);
  m_stop_button.set_sensitive(true);
  
  // clear any previous results
  m_ref_liststore->clear();
  
  m_in_progress = true;
    
  set_status( _("Search in progress...") );
}

void SearchDialog::stop_cb()
{

}

void SearchDialog::add_cb()
{
  using namespace std;

  Glib::RefPtr<Gtk::TreeSelection> refSelection = m_treeview.get_selection();
  Gtk::TreeModel::iterator iter = refSelection->get_selected();

  if(!iter)
    return;
  Gtk::TreeModel::Row row = *iter;
  ICQ2000::ContactRef c = icqclient.getContact(row[m_columns.uin] );
  if (c.get() == NULL)
  {
    ICQ2000::ContactRef nc(new ICQ2000::Contact(row[m_columns.uin] ));
    ICQ2000::ContactTree::Group * gp = ( m_selected_group != NULL ? m_selected_group :
      AddContactDialog::create_new_group() );
    gp->add(nc);

  //	if (m_alert_check.get_active())
  //	{
  //	  ICQ2000::UserAddEvent *ev = new ICQ2000::UserAddEvent(nc);
  //	  icqclient.SendEvent(ev);
  //	}

    // fetch user info
    icqclient.fetchDetailContactInfo(nc);
  }
}

void SearchDialog::reset_cb()
{

}

void SearchDialog::select_row_cb()
{
  m_add_button.set_sensitive(true);
}

void SearchDialog::unselect_row_cb(gint x, gint y, GdkEvent *ev)
{

}

void SearchDialog::set_status(const std::string& s)
{
  //  m_status.pop();
  m_status.push(s);
}

void SearchDialog::set_sex(ICQ2000::Sex s)
{
  m_sex_selected=s;
}

void SearchDialog::set_agerange(ICQ2000::AgeRange age)
{
  m_agerange_selected=age;
}

void SearchDialog::result_cb(ICQ2000::SearchResultEvent *ev)
{
  using namespace std;
  if (m_ev == ev) {
    // our search!!

    if (!m_ev->isExpired()) {
      ContactList& cl = m_ev->getContactList();
      ContactRef c = m_ev->getLastContactAdded();

      if (c.get() != NULL) {
	Gtk::TreeRow row = *(m_ref_liststore->append());
 	row[m_columns.alias]      = Glib::locale_to_utf8( c->getAlias() );
 	row[m_columns.uin]        = c->getUIN();
 	row[m_columns.first_name] = Glib::locale_to_utf8( c->getFirstName() );
 	row[m_columns.last_name]  = Glib::locale_to_utf8( c->getLastName() );
 	row[m_columns.email]      = Glib::locale_to_utf8( c->getEmail() );
 	row[m_columns.auth_req]   = c->getAuthReq();

	//	Gtk::CList_Helpers::RowIterator r = m_clist.rows().insert( m_clist.rows().end(), row_array );
	//	Gtk::ImageLoader *p = g_icons.IconForStatus(c->getStatus(),false);
	//	(*r)[0].set_pixmap( p->pix(), p->bit() );

	/* the ContactRef needs to be dynamically allocated here
	   and deleted on clist destroy, with TreeView too?? */

	//row[m_columns.contact] = c;
	//(*r).set_data( cc, clist_data_destroy_cb );

	//m_clist.columns_autosize();
      }
      
      ostringstream ostr;
      if (m_ev->isFinished()) {
	ostr << _("Search finished, matches found: ") << cl.size();
	if (m_ev->getNumberMoreResults() > 0) {
	  ostr << _(", there are a further ") << m_ev->getNumberMoreResults() << _(" matches not shown.");
	}
	
      } else {
	ostr << _("Search in progress, matches found so far: ") << cl.size();
      }
      set_status( ostr.str() );
    } else {
      set_status( _("Search timeout reached.") );
    }
    
    if (m_ev->isFinished()) {
      m_in_progress = false;
      m_search_button.set_sensitive(true);
      m_stop_button.set_sensitive(false);
      m_ev = NULL;
    }
  }
}

void SearchDialog::self_status_change_cb(ICQ2000::StatusChangeEvent *ev)
{

}

void SearchDialog::selected_group_cb(ICQ2000::ContactTree::Group * gp)
{
  m_selected_group = gp;
}
