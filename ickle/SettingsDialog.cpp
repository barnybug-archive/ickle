/* $Id: SettingsDialog.cpp,v 1.66 2003-03-11 21:30:01 cborni Exp $
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
#include <gtkmm/messagedialog.h>
#include <gtkmm/menu.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/menushell.h>

#include "main.h"
#include "Settings.h"

#include "ickle.h"
#include "UserInfoHelpers.h"
#include "ucompose.h"
#include "utils.h"
#include "Icons.h"


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
  : Gtk::Dialog( _("ickle Preferences"), parent), m_page_title( "", 1.0, 0.5 ),
    m_apply_button(Gtk::Stock::APPLY), m_ok_button(Gtk::Stock::OK)
{
  set_modal(true);
  set_default_size( 600, 400 );
  set_position( Gtk::WIN_POS_CENTER );
  set_border_width(5);

  m_apply_button.signal_clicked().connect( SigC::slot( *this, &SettingsDialog::on_apply_clicked ) );
  m_ok_button.signal_clicked().connect( SigC::slot( *this, &SettingsDialog::on_ok_clicked ) );
  get_action_area()->pack_end( m_apply_button );
  get_action_area()->pack_end( m_ok_button );
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
  
  // only after special changes the client has to be restarted
  m_client_restart=false;
  
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

void SettingsDialog::on_apply_clicked()
{
  if (validate_pages())
  {
    save_pages();
    m_apply_button.set_sensitive(false);
  }
}

void SettingsDialog::on_ok_clicked()
{
  if (validate_pages())
  {
    save_pages();
    response(Gtk::RESPONSE_OK);
  }
}

Gtk::TreeModel::iterator SettingsDialog::add_page(const Glib::ustring& title, Gtk::Widget * page, bool toplevel)
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

  return row;
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
  SectionFrame * login = manage( new SectionFrame( _("Login") ) );
  Gtk::Table * table = manage( new Gtk::Table( 2, 2 ) );
  
  table->attach( * manage( new Gtk::Label( _("UIN"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );
  m_login_uin.set_range(1, 0xffffffff);
  m_login_uin.set_digits(0);
  m_login_uin.set_increments(1, 10);
  m_login_uin.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_login_uin.signal_changed().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_login_uin,_("UIN is the unique number that identifies the user"));
  
  table->attach( m_login_uin, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
  table->attach( * manage( new Gtk::Label( _("Password"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );
  m_login_pass.set_visibility(false);
  m_login_pass.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_login_pass.signal_changed().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_login_pass,_("the password for the account"));
  table->attach( m_login_pass, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL | Gtk::EXPAND );
     
  

  table->set_spacings(5);
  table->set_border_width(10);
  
  login->add( * table );
  
  SectionFrame * autoconnect = manage( new SectionFrame( _("Auto connect") ) );
  table = manage( new Gtk::Table( 2, 1 ) );
  table->attach( * manage( new Gtk::Label( _("Auto connect to"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL );
  
  Gtk::OptionMenu * autoconnect_om = manage (new Gtk::OptionMenu() );
  m_tooltip.set_tip (* autoconnect_om,_("Specifies the state for auto connect"));
  {
    using namespace Gtk::Menu_Helpers;
    Gtk::Menu *menu = Gtk::manage( new Gtk::Menu() );
    MenuList menu_list = menu->items();
    for (int n = ICQ2000::STATUS_ONLINE; n <= ICQ2000::STATUS_OFFLINE; n++)
    {
      Glib::RefPtr<Gdk::Pixbuf> p = g_icons.get_icon_for_status(ICQ2000::Status(n), false );
      Gtk::Image * img = manage( new Gtk::Image( p ) );
      Glib::ustring label = UserInfoHelpers::getStringFromStatus(ICQ2000::Status(n) );
      menu_list.push_back( ImageMenuElem( label,* img, 
      			  SigC::bind<int>( SigC::slot(*this, &SettingsDialog::choose_autoconnect),n) ) );
    }
    //we have to load here, otherwise we have to make the menu a variable of the class
    m_auto_connect=ICQ2000::Status(g_settings.getValueUnsignedInt("autoconnect") );
    menu->set_active(m_auto_connect);
    autoconnect_om->set_menu( *menu );
  }
  table->attach( * autoconnect_om, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL );
  
  table->set_spacings(5);
  table->set_border_width(10);
  
  autoconnect->add( * table );
  
  SectionFrame * reconnect = manage( new SectionFrame( _("Auto reconnect") ) );
  table = manage( new Gtk::Table( 2, 2 ) );
  m_auto_reconnect.set_label(_("Auto reconnect"));
  m_auto_reconnect.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_reconnect,_("Determines if ickle tries to reconnect after an error."));
  table->attach( m_auto_reconnect, 0, 2, 0, 1, Gtk::FILL, Gtk::FILL );
  
  table->attach( * manage( new Gtk::Label( _("Reconnect retries"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
  m_reconnect_retries.set_range(1, 10);
  m_reconnect_retries.set_digits(0);
  m_reconnect_retries.set_increments(1, 1);
  m_reconnect_retries.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_reconnect_retries,_("Specifies how often ickle tries to reconnect after an error."));
  table->attach( m_reconnect_retries, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
  
  table->set_spacings(5);
  table->set_border_width(10);
  
  reconnect->add( * table );
  
  

  vbox->pack_start( *login );
  vbox->pack_start( *autoconnect );
  vbox->pack_start( *reconnect );
  add_page( _("Login"), vbox, true );
}


void SettingsDialog::init_look_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  add_page( _("Look'n'feel"), vbox, true );

  // sub-pages
  init_look_message_page();
  init_look_contact_list_page();
  init_look_icons_page();
  init_look_charset_page();
}

void SettingsDialog::init_look_message_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  SectionFrame * frame = manage( new SectionFrame( _("Messages") ) );
  
  Gtk::Table * table = manage (new Gtk::Table( 2, 4 ) );
  m_message_autoclose.set_label(_("Auto close after sending a message"));
  table->attach( m_message_autoclose,
		 0, 2, 0, 1, Gtk::FILL, Gtk::FILL );
  m_message_autoclose.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_message_autoclose,_("If checked the messagebox is closed after sending a message."));
  
  m_message_autopopup.set_label(_("Auto popup on incoming message"));
  table->attach( m_message_autopopup,
		 0, 2, 1, 2, Gtk::FILL, Gtk::FILL );
  m_message_autopopup.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_message_autopopup,_("If checked the message window will popup in case of  an incoming message."));
  
  m_message_autoraise.set_label(_("Auto raise on incoming message"));
  table->attach( m_message_autoraise,
		 0, 2, 2, 3, Gtk::FILL, Gtk::FILL );
  m_message_autoraise.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_message_autoraise,_("If checked an existing message window will be raised in case of an incoming message."));
  
  table->set_spacings(5);
  table->set_border_width(10);

  frame->add( * table );
  
  SectionFrame * history = manage( new SectionFrame( _("History") ) );
  
  table = manage (new Gtk::Table( 2, 1 ) );
  
  table->attach( * manage( new Gtk::Label( _("Messages per page"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );
  
  m_history_shownr.set_range(0, 0xff);
  m_history_shownr.set_digits(0); 
  m_history_shownr.set_increments(1, 10);
  m_history_shownr.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_history_shownr,_("Defines the number of messages per page in the message window history"));
  table->attach( m_history_shownr, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
  
  table->set_spacings(5);
  table->set_border_width(10);
  
  history->add( * table );
  

  vbox->pack_start( *frame );
  vbox->pack_start( *history );
  add_page( _("Message window"), vbox, false );
}

void SettingsDialog::init_look_contact_list_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  add_page( _("Contact list"), vbox, false );
}

void SettingsDialog::init_look_charset_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();

  SectionFrame * frame = manage( new SectionFrame( _("Character set") ) );

  Gtk::VBox * vbox2 = manage( new Gtk::VBox() );
  
  Gtk::Table * table = manage(new Gtk::Table( 3, 1 ) );

  table->set_spacings(5);
  table->set_border_width(10);
  
  table->attach( * manage( new Gtk::Label( _("Default"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );

  m_lnf_charset.signal_changed().connect( SigC::slot( *this, &SettingsDialog::lnf_charset_validate_cb ) );
  m_lnf_charset.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  table->attach( m_lnf_charset, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );

  m_lnf_charset_valid.set_markup( _("<b>Valid</b>") );
  table->attach( m_lnf_charset_valid, 2, 3, 0, 1, Gtk::FILL, Gtk::FILL );

  vbox2->pack_start( * table, Gtk::PACK_SHRINK );

  Gtk::Label * label = manage( new Gtk::Label() );
  
  label->set_markup( _("This setting determines the global default character set used when sending and receiving "
		       "to the <i>ICQ</i> network. You can set per-contact encodings by choosing the relevant option "
		       "from the right-click menu for a contact.\n\n"
		       "You should use a character set name supported by iconv. "
		       "If you are using GNU <tt>iconv</tt>, you should be able to list the available encodings "
		       "by running the command:\n"
		       "<tt>iconv --list</tt>\n\n") );
  label->set_line_wrap(true);
  label->set_justify(Gtk::JUSTIFY_FILL);

  vbox2->pack_start( * label, Gtk::PACK_SHRINK );

  frame->add( * vbox2 );

  vbox->pack_start( *frame );
  
  m_row_lnf_charset = add_page( _("Character set"), vbox, false );
}

void SettingsDialog::init_look_icons_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  add_page( _("Icons"), vbox, false );
}

void SettingsDialog::init_away_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  add_page( _("Away"), vbox, true );
  
  // sub-pages
  init_away_idle_page();
  init_away_message_page();
}

void SettingsDialog::init_away_idle_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  
  SectionFrame * frame = manage( new SectionFrame( _("Away/Idle") ) );
  
  Gtk::Table * table = manage( new Gtk::Table( 2, 3 ) );

  table->attach( * manage( new Gtk::Label( _("Auto-away (minutes)"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );
  
  m_auto_away.set_range(0, 0xffff);
  m_auto_away.set_digits(0); 
  m_auto_away.set_increments(1, 10);
  m_auto_away.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_away,_("After this time ickle will switch its mode to away. 0 to disable"));
  table->attach( m_auto_away, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
    
  table->attach( * manage( new Gtk::Label( _("Auto-N/A (minutes)"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );
		 
  m_auto_na.set_range(0, 0xffff);
  m_auto_na.set_digits(0); 
  m_auto_na.set_increments(1, 10);
  m_auto_na.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_na,_("After this time ickle will switch its mode to Not Available. 0 to disable"));
  table->attach( m_auto_na, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL );

  
  m_auto_return.set_label(_("Automatically return (from Auto-away or Auto-N/A)"));
  m_auto_return.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_return,_("Determines if ickle will return from Auto-away or Auto-N/A after pressing a key or moving the mouse."));
  table->attach( m_auto_return,0,2,2,3, Gtk::FILL, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);
  
  frame->add( * table );

  vbox->pack_start( *frame );
  add_page( _("Idle"), vbox, false );
  
  
  
}

void SettingsDialog::init_away_message_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  add_page( _("Away Messages"), vbox, false );
}

void SettingsDialog::init_advanced_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  add_page( _("Advanced"), vbox, true );

  // sub-pages
  init_advanced_security_page();
  init_advanced_server_page();
  init_advanced_smtp_page();
  init_advanced_logging_page();
}

void SettingsDialog::init_advanced_security_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  add_page( _("Security"), vbox, false );
}

void SettingsDialog::init_advanced_server_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();

  SectionFrame * frame = manage( new SectionFrame( _("Server") ) );
  
  Gtk::Table * table = manage( new Gtk::Table( 2, 2 ) );

  table->attach( * manage( new Gtk::Label( _("Server name"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );
  
  m_network_login_host.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_network_login_host,_("Specifies the name of the login server"));
  table->attach( m_network_login_host, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
    
  table->attach( * manage( new Gtk::Label( _("Server port"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );
  m_network_login_port.set_range(0, 0xffff);
  m_network_login_port.set_digits(0); 
  m_network_login_port.set_increments(1, 10);
  m_network_login_port.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_network_login_port,_("Specifies the port of the login server. Default is 5190"));
  table->attach( m_network_login_port, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL );
  
  table->set_spacings(5);
  table->set_border_width(10);
  
  frame->add( * table );

  vbox->pack_start( *frame );
  add_page( _("Server"), vbox, false );
}
void SettingsDialog::init_advanced_smtp_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  add_page( _("SMTP"), vbox, false );
}
void SettingsDialog::init_advanced_logging_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  add_page( _("Logging"), vbox, false );
}

void SettingsDialog::load_pages()
{
  load_login_page();
  load_look_page();
  load_away_page();
  load_advanced_page();
  m_apply_button.set_sensitive(false);
}


void SettingsDialog::load_login_page()
{
  m_login_uin.set_value((double)g_settings.getValueUnsignedInt("uin") );
  m_login_pass.set_text( g_settings.getValueString("password") );
  m_auto_connect=ICQ2000::Status(g_settings.getValueUnsignedInt("autoconnect") );
  m_auto_reconnect.set_active(g_settings.getValueBool("auto_reconnect") );
  m_reconnect_retries.set_value((double)g_settings.getValueUnsignedInt("reconnect_retries") );

}

void SettingsDialog::load_look_page()
{
  load_look_message_page();
  load_look_contact_list_page();
  load_look_icons_page();
  load_look_charset_page();
}

void SettingsDialog::load_look_message_page()
{
  m_message_autoclose.set_active(g_settings.getValueBool("message_autoclose") );
  m_message_autopopup.set_active(g_settings.getValueBool("message_autopopup") );
  m_message_autoraise.set_active(g_settings.getValueBool("message_autoraise") );
  m_history_shownr.set_value((double)g_settings.getValueUnsignedInt("history_shownr") );
}

void SettingsDialog::load_look_contact_list_page()
{
}

void SettingsDialog::load_look_icons_page()
{
}

void SettingsDialog::load_look_charset_page()
{
  m_lnf_charset.set_text( g_settings.getValueString("encoding") );
}

void SettingsDialog::load_away_page()
{
  load_away_idle_page();
  load_away_message_page();
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
  load_advanced_security_page();
  load_advanced_server_page();
  load_advanced_smtp_page();
  load_advanced_logging_page();
}

void SettingsDialog::load_advanced_security_page()
{
}

void SettingsDialog::load_advanced_server_page()
{
  m_network_login_host.set_text( g_settings.getValueString("network_login_host") );
  m_network_login_port.set_value((double) g_settings.getValueUnsignedInt("network_login_port") );
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
  save_away_page();
  save_advanced_page();
}

//Saves the settings of the login page
void SettingsDialog::save_login_page()
{
  g_settings.setValue( "uin", (unsigned int) m_login_uin.get_value() );
  g_settings.setValue( "password", m_login_pass.get_text() );
  g_settings.setValue( "autoconnect",(unsigned int) m_auto_connect );
  g_settings.setValue( "autoreconnect", m_auto_reconnect.get_active() );
  g_settings.setValue( "reconnect_retries", (unsigned int) m_reconnect_retries.get_value() );
}


//Saves the settings of the look page
void SettingsDialog::save_look_page()
{
  save_look_message_page();
  save_look_contact_list_page();
  save_look_icons_page();
  save_look_charset_page();
}

void SettingsDialog::save_look_message_page()
{
  g_settings.setValue("message_autoclose", m_message_autoclose.get_active() );
  g_settings.setValue("message_autopopup", m_message_autopopup.get_active() );
  g_settings.setValue("message_autoraise", m_message_autoraise.get_active() );    
  g_settings.setValue( "history_shownr", (unsigned int) m_history_shownr.get_value() );
}

void SettingsDialog::save_look_contact_list_page()
{
}

void SettingsDialog::save_look_icons_page()
{
}

void SettingsDialog::save_look_charset_page()
{
  g_settings.setValue( "encoding", m_lnf_charset.get_text() );
}




//Saves the settings of the look page
void SettingsDialog::save_away_page()
{
  save_away_idle_page();
  save_away_message_page();
}

void SettingsDialog::save_away_idle_page()
{
  g_settings.setValue( "auto_away", (unsigned int) m_auto_away.get_value() );
  g_settings.setValue( "auto_na", (unsigned int) m_auto_na.get_value() );
  g_settings.setValue( "auto_return", m_auto_return.get_active() );
}

void SettingsDialog::save_away_message_page()
{
}



//Saves the settings of the look page
void SettingsDialog::save_advanced_page()
{
  save_advanced_security_page();
  save_advanced_server_page();
  save_advanced_smtp_page();
  save_advanced_logging_page();
}

void SettingsDialog::save_advanced_security_page()
{
}

void SettingsDialog::save_advanced_server_page()
{
g_settings.setValue( "network_login_host", m_network_login_host.get_text() );
g_settings.setValue( "network_login_port", (unsigned int) m_network_login_port.get_value() );

}


void SettingsDialog::save_advanced_smtp_page()
{
}

void SettingsDialog::save_advanced_logging_page()
{
}

void SettingsDialog::changed_cb()
{
  m_apply_button.set_sensitive(true);
}

void SettingsDialog::client_changed()
{
  m_client_restart=true;
}

void SettingsDialog::lnf_charset_validate_cb()
{
  m_lnf_charset_valid.set_sensitive( Utils::is_valid_encoding(m_lnf_charset.get_text() ) );
}

bool SettingsDialog::validate_pages()
{
  bool valid = true;
  
  if (valid)
  {
    valid = validate_look_charset_page();
  }

  return valid;
}

bool SettingsDialog::validate_look_charset_page()
{
  if ( ! Utils::is_valid_encoding(m_lnf_charset.get_text()) )
  {
    m_lnf_charset.grab_focus();
    m_page_tree.get_selection()->select(m_row_lnf_charset);

    Gtk::MessageDialog dialog (*this,
			       String::ucompose( _("\"%1\" is not a valid encoding on your system."), m_lnf_charset.get_text()),
			       Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    
    dialog.run();

    return false;
  }

  return true;
}


void SettingsDialog::choose_autoconnect (unsigned int s) 
{
  m_auto_connect=ICQ2000::Status(s);
  changed_cb();
}
