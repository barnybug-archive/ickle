/* $Id: SettingsDialog.cpp,v 1.59 2003-02-06 19:01:15 barnabygray Exp $
 *
 * Copyright (C) 2001-2003 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#include "SettingsDialog.h"

#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/stock.h>

#include "main.h"
#include "Settings.h"

#include "ickle.h"
#include "ucompose.h"

class SectionFrame : public Gtk::Frame
{
 public:
  SectionFrame(const Glib::ustring& label);
};

SectionFrame::SectionFrame(const Glib::ustring& label)
{
  Gtk::Label * l = manage( new Gtk::Label() );
  l->set_markup( String::ucompose("<span weight=\"bold\">%1</span>", label) );
  set_label_widget( *l );
  set_border_width( 10 );
  set_shadow_type( Gtk::SHADOW_NONE );
}

SettingsDialog::SettingsDialog(Gtk::Window& parent, bool start_on_away)
  : Gtk::Dialog( _("ickle Preferences"), parent), m_page_title( "", 1.0, 0.5 )
{
  set_modal(true);
  set_default_size( 600, 400 );
  set_position( Gtk::WIN_POS_CENTER );
  set_border_width(5);

  m_apply_button = add_button(Gtk::Stock::APPLY,  Gtk::RESPONSE_APPLY);
  add_button(Gtk::Stock::OK,     Gtk::RESPONSE_OK);
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  Gtk::HBox * top_hbox;
  Gtk::VBox * top_vbox = get_vbox();
  Gtk::Frame * frame;
  
  top_hbox = manage( new Gtk::HBox() );

  // setup Tree store
  m_reftreestore = Gtk::TreeStore::create(m_columns);
  
  // setup Tree view
  m_page_tree.set_model( m_reftreestore );
  m_page_tree.set_headers_visible( false );
  m_page_tree.get_selection()->signal_changed().connect( SigC::slot( *this, &SettingsDialog::page_tree_select_cb ) );

  // create column
  m_page_tree.append_column( "", m_columns.label );

  frame = manage( new Gtk::Frame() );
  frame->set_shadow_type( Gtk::SHADOW_IN );
  frame->add( m_page_tree );

  top_hbox->pack_start( *frame, Gtk::PACK_SHRINK );

  frame = manage( new Gtk::Frame() );
  frame->set_shadow_type( Gtk::SHADOW_OUT );
  m_page_title.set_padding( 10, 3 );
  
  frame->add( m_page_title );
  
  Gtk::VBox * vbox = manage( new Gtk::VBox() );

  vbox->pack_start( *frame, Gtk::PACK_SHRINK );

  m_main_frame.set_shadow_type( Gtk::SHADOW_NONE );
  vbox->pack_start( m_main_frame, Gtk::PACK_EXPAND_WIDGET );

  frame = manage( new Gtk::Frame() );
  frame->set_shadow_type( Gtk::SHADOW_IN );
  frame->set_border_width( 3 );
  frame->add( *vbox );

  top_hbox->pack_start( * frame,      Gtk::PACK_EXPAND_WIDGET );

  top_vbox->pack_start( * top_hbox );
		    
  // ==================================================
  //  Login
  // ==================================================

  init_pages();
  load_pages();
  
  // finally show all!

  show_all();
  
}

SettingsDialog::~SettingsDialog()
{
  Gtk::TreeModel::Children ch = m_reftreestore->children();
  Gtk::TreeModel::iterator iter = ch.begin();
  while ( iter != ch.end() )
  {
    delete (*iter)[ m_columns.widget ];
    ++iter;
  }
}

void SettingsDialog::add_page(const Glib::ustring& title, Gtk::Widget * page, bool toplevel)
{
  Gtk::TreeModel::Row row;
  
  if (toplevel)
  {
    row = * m_reftreestore->append();
  }
  else
  {
    Gtk::TreeModel::iterator iter = m_reftreestore->children()[ m_reftreestore->children().size()-1 ];
    row = * m_reftreestore->append( iter->children() );
  }
  row[ m_columns.label ] = title;
  row[ m_columns.widget ] = page;
}

void SettingsDialog::page_tree_select_cb()
{
  Glib::RefPtr<Gtk::TreeView::Selection> sel = m_page_tree.get_selection();
  Gtk::TreeModel::iterator iter = sel->get_selected();
  if (iter)
  {

    Gtk::TreeModel::Row row = *iter;
    
    m_page_title.set_markup( String::ucompose("<span weight=\"bold\" size=\"larger\">%1</span>", row[ m_columns.label ]) );
    
    m_main_frame.remove();
    m_main_frame.add( * row[ m_columns.widget ] );
    m_main_frame.show_all();
  }
  
}

void SettingsDialog::init_pages()
{
  init_login_page();
  init_look_page();
  init_away_page();
  init_advanced_page();

  m_page_tree.expand_all();
}

void SettingsDialog::init_login_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  
  SectionFrame * frame = manage( new SectionFrame( _("UIN/Password") ) );
  
  Gtk::Table * table = new Gtk::Table( 2, 2 );

  table->attach( * manage( new Gtk::Label( _("UIN"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );
  m_login_uin.set_range(1, 0xffffffff);
  m_login_uin.set_digits(0);
  m_login_uin.set_increments(1, 10);
  m_login_uin.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_login_uin,"UIN is the unique number that identifies the user");
  
  table->attach( m_login_uin, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
  table->attach( * manage( new Gtk::Label( _("Password"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );
  m_login_pass.set_visibility(false);
  m_login_pass.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_login_pass,"the password for the account");
  table->attach( m_login_pass, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);
  
  frame->add( * table );

  vbox->pack_start( *frame );
  
  add_page( _("Login"), vbox, true );
}

void SettingsDialog::init_look_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("Look'n'feel"), vbox, true );

  // sub-pages
  init_look_message_page();
  init_look_contact_list_page();
  init_look_icons_page();
}

void SettingsDialog::init_look_message_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("Messages"), vbox, false );
}

void SettingsDialog::init_look_contact_list_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("Contact list"), vbox, false );
}

void SettingsDialog::init_look_icons_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("Icons"), vbox, false );
}

void SettingsDialog::init_away_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("Away"), vbox, true );
  
  // sub-pages
  init_away_idle_page();
  init_away_message_page();
}

void SettingsDialog::init_away_idle_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  
  SectionFrame * frame = manage( new SectionFrame( _("Away/Idle") ) );
  
  Gtk::Table * table = new Gtk::Table( 2, 3 );

  table->attach( * manage( new Gtk::Label( _("Auto-away (minutes)"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );
  
  m_auto_away.set_range(0, 0xffff);
  m_auto_away.set_digits(0); 
  m_auto_away.set_increments(1, 10);
  m_auto_away.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_away,"After this time ickle will switch its mode to away. 0 to disable");
  table->attach( m_auto_away, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
    
  table->attach( * manage( new Gtk::Label( _("Auto-N/A (minutes)"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );
		 
  m_auto_na.set_range(0, 0xffff);
  m_auto_na.set_digits(0); 
  m_auto_na.set_increments(1, 10);
  m_auto_na.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_na,"After this time ickle will switch its mode to Not Available. 0 to disable");
  table->attach( m_auto_na, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL );

  
  m_auto_return.set_label("Automatically return (from Auto-away or Auto-N/A)");
  m_auto_return.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_return,"Determines if ickle will return from Auto-away or Auto-N/A after pressing a key or moving the mouse.");
  table->attach( m_auto_return,0,2,2,3, Gtk::FILL, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);
  
  frame->add( * table );

  vbox->pack_start( *frame );
  add_page( _("Idle"), vbox, false );
  
  
  
}

void SettingsDialog::init_away_message_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("Away Messages"), vbox, false );
}

void SettingsDialog::init_advanced_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("Advanced"), vbox, true );

  // sub-pages
  init_advanced_security_page();
  init_advanced_smtp_page();
  init_advanced_logging_page();
}

void SettingsDialog::init_advanced_security_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("Security"), vbox, false );
}
void SettingsDialog::init_advanced_smtp_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("SMTP"), vbox, false );
}
void SettingsDialog::init_advanced_logging_page()
{
  Gtk::VBox * vbox = manage( new Gtk::VBox() );
  add_page( _("Logging"), vbox, false );
}

void SettingsDialog::load_pages()
{
  load_login_page();
  load_look_page();
  load_look_message_page();
  load_look_contact_list_page();
  load_look_icons_page();
  load_away_page();
  load_away_idle_page();
  load_away_message_page();
  load_advanced_page();
  load_advanced_security_page();
  load_advanced_smtp_page();
  load_advanced_logging_page();

  m_apply_button->set_sensitive(false);
}

void SettingsDialog::load_login_page()
{
  m_login_uin.set_value((double)g_settings.getValueUnsignedInt("uin") );
  m_login_pass.set_text( g_settings.getValueString("password") );
}

void SettingsDialog::load_look_page()
{
}

void SettingsDialog::load_look_message_page()
{
}

void SettingsDialog::load_look_contact_list_page()
{
}

void SettingsDialog::load_look_icons_page()
{
}

void SettingsDialog::load_away_page()
{
}

void SettingsDialog::load_away_idle_page()
{
  m_auto_away.set_value((double)g_settings.getValueUnsignedInt("auto_away") );
  m_auto_na.set_value((double)g_settings.getValueUnsignedInt("auto_na") );
  m_auto_return.set_active(g_settings.getValueBool("auto_return") );
}

void SettingsDialog::load_away_message_page()
{
}

void SettingsDialog::load_advanced_page()
{
}

void SettingsDialog::load_advanced_security_page()
{
}

void SettingsDialog::load_advanced_smtp_page()
{
}

void SettingsDialog::load_advanced_logging_page()
{
}


void SettingsDialog::save_pages()
{
  save_login_page();
  save_look_page();
  save_look_message_page();
  save_look_contact_list_page();
  save_look_icons_page();
  save_away_page();
  save_away_idle_page();
  save_away_message_page();
  save_advanced_page();
  save_advanced_security_page();
  save_advanced_smtp_page();
  save_advanced_logging_page();
}

void SettingsDialog::save_login_page()
{
}

void SettingsDialog::save_look_page()
{
}

void SettingsDialog::save_look_message_page()
{
}

void SettingsDialog::save_look_contact_list_page()
{
}

void SettingsDialog::save_look_icons_page()
{
}

void SettingsDialog::save_away_page()
{
}

void SettingsDialog::save_away_idle_page()
{
}

void SettingsDialog::save_away_message_page()
{
}

void SettingsDialog::save_advanced_page()
{
}

void SettingsDialog::save_advanced_security_page()
{
}

void SettingsDialog::save_advanced_smtp_page()
{
}

void SettingsDialog::save_advanced_logging_page()
{
}

void SettingsDialog::changed_cb()
{
  m_apply_button->set_sensitive(true);
}

void SettingsDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_APPLY
      || response_id == Gtk::RESPONSE_OK)
  {
    // apply stuff
  }

  if (response_id == Gtk::RESPONSE_APPLY)
  {
    return;
  }
  else
  {
    Dialog::on_response(response_id);
  }
}
