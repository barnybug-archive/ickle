/* $Id: SettingsDialog.cpp,v 1.79 2004-02-15 21:14:27 cborni Exp $
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
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/fontselection.h>

#include "main.h"
#include "Settings.h"
#include "PromptDialog.h"

#include "ickle.h"
#include "UserInfoHelpers.h"
#include "ucompose.h"
#include "utils.h"
#include "Icons.h"

#include <iostream>
#include <vector>

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

  //same for the contact list
  m_icons_changed=false;

  //same for the fonts
  m_fonts_changed=false;

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
    if (m_client_restart)
    {
      activate_changes();
      m_client_restart=false;
    }
    if (m_icons_changed)
    {
      g_icons.setIcons(m_icons_dir);
    }
    if (m_fonts_changed)
    {
      m_fonts_changed=false;
      change_fonts.emit();
    }
  }
}

void SettingsDialog::on_ok_clicked()
{
  if (validate_pages())
  {
    save_pages();
    if (m_client_restart)
    {
      activate_changes();
      m_client_restart=false;
    }
    if (m_icons_changed)
    {
      g_icons.setIcons(m_icons_dir);
    }
    if (m_fonts_changed)
    {
      m_fonts_changed=false;
      change_fonts.emit();
    }
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
  init_events_page();
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
  m_tooltip.set_tip (m_login_uin,_("UIN is your unique number that identifies you on ICQ"));

  table->attach( m_login_uin, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
  table->attach( * manage( new Gtk::Label( _("Password"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );
  m_login_pass.set_visibility(false);
  m_login_pass.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_login_pass.signal_changed().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_login_pass,_("The password for your account"));
  table->attach( m_login_pass, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL | Gtk::EXPAND );



  table->set_spacings(5);
  table->set_border_width(10);

  login->add( * table );

  SectionFrame * autoconnect = manage( new SectionFrame( _("Auto connect") ) );
  table = manage( new Gtk::Table( 2, 1 ) );
  table->attach( * manage( new Gtk::Label( _("Auto connect to"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL );

  Gtk::OptionMenu * autoconnect_om = manage (new Gtk::OptionMenu() );
  m_tooltip.set_tip (* autoconnect_om,_("Specifies the status to connect as when ickle is first loaded"));
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
  m_auto_reconnect.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::toggle_reconnect ) );
  m_tooltip.set_tip (m_auto_reconnect,_("Determines if ickle will attempt to reconnect when disconnected prematurely"));
  table->attach( m_auto_reconnect, 0, 2, 0, 1, Gtk::FILL, Gtk::FILL );

  table->attach( * manage( new Gtk::Label( _("Reconnect retries"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
  m_reconnect_retries.set_range(1, 10);
  m_reconnect_retries.set_digits(0);
  m_reconnect_retries.set_increments(1, 1);
  m_reconnect_retries.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_reconnect_retries,_("Specifies how many attempts ickle will make at reconnecting"));
  table->attach( m_reconnect_retries, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);

  table->set_spacings(5);
  table->set_border_width(10);

  reconnect->add( * table );



  vbox->pack_start( *login, Gtk::PACK_SHRINK );
  vbox->pack_start( *autoconnect, Gtk::PACK_SHRINK );
  vbox->pack_start( *reconnect, Gtk::PACK_SHRINK );
  add_page( _("Login"), vbox, true );
}


void SettingsDialog::init_look_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  SectionFrame * fonts = manage( new SectionFrame( _("Messagebox fonts") ) );

  Gtk::Table * table = manage (new Gtk::Table( 2, 2 ) );
  table->attach( * manage( new Gtk::Label( _("Header font"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );

  m_message_header_font.signal_clicked().connect( SigC::slot( *this, &SettingsDialog::set_message_header_font_cb ) );
  m_message_header_font.signal_clicked().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  table->attach( m_message_header_font, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );

  table->attach( * manage( new Gtk::Label( _("Text font"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );

  m_message_text_font.signal_clicked().connect( SigC::slot( *this, &SettingsDialog::set_message_text_font_cb ) );
  m_message_text_font.signal_clicked().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  table->attach( m_message_text_font, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL );

  fonts->add( * table );
  vbox->pack_start( *fonts, Gtk::PACK_SHRINK );

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
  m_tooltip.set_tip (m_message_autoclose,_("If checked the message window is closed after sending a message"));

  m_message_autopopup.set_label(_("Auto popup on incoming message"));
  table->attach( m_message_autopopup,
		 0, 2, 1, 2, Gtk::FILL, Gtk::FILL );
  m_message_autopopup.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_message_autopopup,_("If checked the message window will popup when an incoming message is received"));

  m_message_autoraise.set_label(_("Auto raise on incoming message"));
  table->attach( m_message_autoraise,
		 0, 2, 2, 3, Gtk::FILL, Gtk::FILL );
  m_message_autoraise.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_message_autoraise,_("If checked open message windows for contacts will be raised on an incoming message"));

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


  vbox->pack_start( *frame, Gtk::PACK_SHRINK );
  vbox->pack_start( *history, Gtk::PACK_SHRINK );
  add_page( _("Message window"), vbox, false );
}

void SettingsDialog::init_look_contact_list_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  SectionFrame * clicking = manage( new SectionFrame( _("Clicking") ) );
  //todo: Offline Contacts
  Gtk::Table * table = manage( new Gtk::Table( 1, 3 ) );

  m_mouse_check_away_click.set_label(_("Check away message with click on icon"));
  m_mouse_check_away_click.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_mouse_check_away_click,_("Single clicking the icon for a contact on your contact list will check their away message (TODO)"));
  table->attach( m_mouse_check_away_click, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );

  m_mouse_single_click.set_label(_("Open message window with single click"));
  m_mouse_single_click.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_mouse_single_click,_("Single clicking on a contact on your contact list will open a message window for them (TODO)"));
  table->attach( m_mouse_single_click, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);

  clicking->add( * table );

  vbox->pack_start( *clicking );
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
  SectionFrame * icons = manage( new SectionFrame( _("Icons") ) );

  Gtk::ScrolledWindow * scr_win = manage( new Gtk::ScrolledWindow() );
  scr_win->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  scr_win->set_shadow_type(Gtk::SHADOW_IN);
  scr_win->set_border_width(10);

  m_icons_treeview.set_headers_visible(false);
  
  std::vector<std::string> iconsets = g_icons.get_icon_sets();
  Glib::RefPtr< Gtk::ListStore > liststore = Gtk::ListStore::create( m_icons_columns );
  m_icons_reftreemodel = liststore;
  m_icons_treeview.set_model( m_icons_reftreemodel );
  m_icons_treeview.get_selection()->signal_changed().connect( SigC::slot( *this, &SettingsDialog::icons_changed) );

  scr_win->add( m_icons_treeview );

  Gtk::TreeView::Column* pColumn = Gtk::manage( new Gtk::TreeView::Column("") );
  pColumn->pack_start( m_icons_columns.icon_online, false );
  pColumn->pack_start( m_icons_columns.icon_away, false );
  pColumn->pack_start( m_icons_columns.icon_na, false );
  pColumn->pack_start( m_icons_columns.icon_occupied, false );
  pColumn->pack_start( m_icons_columns.icon_dnd, false );
  pColumn->pack_start( m_icons_columns.icon_ffc, false );
  pColumn->pack_start( m_icons_columns.icon_offline, false );
  pColumn->pack_start( m_icons_columns.icon_invisible, false );
  m_icons_treeview.append_column(*pColumn);

  m_icons_treeview.append_column("", m_icons_columns.name);

  /* now add the rows */
  for ( std::vector< std::string >::const_iterator curr = iconsets.begin();
	curr != iconsets.end() ;
	++curr )
  {
    Gtk::TreeModel::Row row = *(liststore->append());
    std::string dir = *curr + "/";
    std::string::size_type n = curr->rfind('/');
    if (n != std::string::npos) ++n;
    std::string name = curr->substr(n);

    row[m_icons_columns.icon_online]    = g_icons.get_icon_for_status( ICQ2000::STATUS_ONLINE,      dir, false );
    row[m_icons_columns.icon_away]      = g_icons.get_icon_for_status( ICQ2000::STATUS_AWAY,        dir, false );
    row[m_icons_columns.icon_na]        = g_icons.get_icon_for_status( ICQ2000::STATUS_NA,          dir, false );
    row[m_icons_columns.icon_occupied]  = g_icons.get_icon_for_status( ICQ2000::STATUS_OCCUPIED,    dir, false );
    row[m_icons_columns.icon_dnd]       = g_icons.get_icon_for_status( ICQ2000::STATUS_DND,         dir, false );
    row[m_icons_columns.icon_ffc]       = g_icons.get_icon_for_status( ICQ2000::STATUS_FREEFORCHAT, dir, false );
    row[m_icons_columns.icon_offline]   = g_icons.get_icon_for_status( ICQ2000::STATUS_OFFLINE,     dir, false );
    row[m_icons_columns.icon_invisible] = g_icons.get_icon_for_status( ICQ2000::STATUS_ONLINE,      dir, true );

    row[m_icons_columns.name] = name;
  }
  
  icons->add( * scr_win );

  vbox->pack_start( *icons );
  add_page( _("Icons"), vbox, false );
}


void SettingsDialog::init_events_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  SectionFrame * commands = manage( new SectionFrame( _("Commands") ) );
  Gtk::Table * table = manage( new Gtk::Table( 2, 6 ) );

  table->attach( * manage( new Gtk::Label( _("Message events"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );
  m_event_message.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_event_message,_("todo"));
  table->attach( m_event_message, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );

  table->attach( * manage( new Gtk::Label( _("SMS events"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );
  m_event_sms.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_event_sms,_("todo"));
  table->attach( m_event_sms, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );

  table->attach( * manage( new Gtk::Label( _("System events"), 0.0, 0.5 ) ),
		 0, 1, 2, 3, Gtk::FILL , Gtk::FILL );
  m_event_system.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_event_system,_("todo"));
  table->attach( m_event_system, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );

  table->attach( * manage( new Gtk::Label( _("URL events"), 0.0, 0.5 ) ),
		 0, 1, 3, 4, Gtk::FILL, Gtk::FILL );
  m_event_url.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_event_url,_("todo"));
  table->attach( m_event_url, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );

  table->attach( * manage( new Gtk::Label( _("User online events"), 0.0, 0.5 ) ),
		 0, 1, 4, 5, Gtk::FILL, Gtk::FILL );
  m_event_user_online.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_event_user_online,_("todo"));
  table->attach( m_event_user_online, 1, 2, 4, 5, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );

  m_substitutions.set_label(_("Show Substitutions") );
  m_substitutions.signal_clicked().connect( SigC::slot (*this, &SettingsDialog::subs_cb ) );
  table->attach (m_substitutions, 0, 2, 5, 6, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);

  table->set_spacings(5);
  table->set_border_width(10);

  commands->add( * table );


  
  SectionFrame * repetition = manage( new SectionFrame( _("Repetition") ) );
  table = manage( new Gtk::Table( 2, 2 ) );

  m_event_repetition_threshold.set_range(0, 10000);
  m_event_repetition_threshold.set_digits(0);
  m_event_repetition_threshold.set_increments(1, 100);
      
  m_event_repetition_threshold.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_event_repetition_threshold,_("Filters out events occouring in quick succession"));
  table->attach( m_event_repetition_threshold , 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND );
  table->attach( * manage( new Gtk::Label( _("Event Repetition Threshold in ms"), 0.0, 0.5 ) ),
		                    0, 1, 0, 1, Gtk::FILL, Gtk::FILL );

  m_event_execute_all.set_label(_("Execute all events") );
  m_event_execute_all.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_event_execute_all,_("Always executes the commands above, even if they're below the threshold. You can then use the %r substitution to check for repeated events."));
  table->attach( m_event_execute_all, 0, 2, 1, 2, Gtk::FILL, Gtk::FILL | Gtk::EXPAND );

  repetition->add( * table );

  vbox->pack_start( *commands, Gtk::PACK_EXPAND_WIDGET );
  vbox->pack_start( *repetition, Gtk::PACK_SHRINK );
  add_page( _("Events"), vbox, true );
}

void SettingsDialog::init_away_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();

  SectionFrame * frame = manage( new SectionFrame( _("Response dialog") ) );

  Gtk::Table * table = manage(new Gtk::Table( 1, 1 ) );
  //todo

  table->set_spacings(5);
  table->set_border_width(10);

  frame->add( * table );

  vbox->pack_start( *frame, Gtk::PACK_SHRINK );

  frame = manage( new SectionFrame( _("Invisibility") ) );

  table = manage(new Gtk::Table( 1, 1 ) );
  //todo

  table->set_spacings(5);
  table->set_border_width(10);

  frame->add( * table );

  vbox->pack_start( *frame, Gtk::PACK_SHRINK );

  add_page( _("Away"), vbox, true );

  // sub-pages
  init_away_idle_page();
  init_away_message_page();
}

void SettingsDialog::init_away_idle_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();

  SectionFrame * frame = manage( new SectionFrame( _("Idleing") ) );

  Gtk::Table * table = manage( new Gtk::Table( 3, 2 ) );

  table->attach( * manage( new Gtk::Label( _("Auto-away"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );

  m_auto_away.set_range(0, 0xffff);
  m_auto_away.set_digits(0);
  m_auto_away.set_increments(1, 10);
  m_auto_away.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_away,_("After this length of idle time ickle will switch to away"));
  table->attach( m_auto_away, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );
  table->attach( * manage( new Gtk::Label( _("minutes"), 0.0, 0.5 ) ),
		 2, 3, 0, 1, Gtk::FILL, Gtk::FILL );

  table->attach( * manage( new Gtk::Label( _("Auto-N/A"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );

  m_auto_na.set_range(0, 0xffff);
  m_auto_na.set_digits(0);
  m_auto_na.set_increments(1, 10);
  m_auto_na.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_na,_("After this length of idle time ickle will switch to N/A"));
  table->attach( m_auto_na, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL );
  table->attach( * manage( new Gtk::Label( _("minutes"), 0.0, 0.5 ) ),
		 2, 3, 1, 2, Gtk::FILL, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);

  frame->add( * table );

  vbox->pack_start( *frame, Gtk::PACK_SHRINK );

  frame = manage( new SectionFrame( _("Returning") ) );

  table = manage( new Gtk::Table( 2, 1 ) );

  m_auto_return.set_label(_("Automatically return"));
  m_auto_return.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_auto_return,_("Determines if ickle will return from auto-away or auto-N/A when idleing is over"));
  table->attach( m_auto_return, 0, 2, 0, 1, Gtk::FILL, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);

  frame->add( * table );

  vbox->pack_start( *frame, Gtk::PACK_SHRINK );

  add_page( _("Idle"), vbox, false );



}

void SettingsDialog::init_away_message_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  
  SectionFrame * frame = manage( new SectionFrame( _("Preset messages") ) );

  Gtk::HBox * hbox = manage( new Gtk::HBox() );
  hbox->set_border_width(10);
  
  // setup the treeview (list)

  Gtk::ScrolledWindow * scr_win = manage( new Gtk::ScrolledWindow() );
  scr_win->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  scr_win->set_shadow_type(Gtk::SHADOW_IN);
  scr_win->set_size_request(140, 100);

  m_away_treeview.set_headers_visible(false);
  
  m_away_refliststore = Gtk::ListStore::create( m_away_columns );
  m_away_treeview.set_model( m_away_refliststore );
  m_away_treeview.get_selection()->signal_changed().connect( SigC::slot( *this, &SettingsDialog::away_message_select_cb) );

  m_away_treeview.append_column_editable("", m_away_columns.label);

  scr_win->add( m_away_treeview );

  hbox->pack_start( * scr_win, Gtk::PACK_SHRINK );

  Gtk::VBox * vbox2 = manage( new Gtk::VBox() );
  vbox2->set_spacing(5);
  vbox2->set_border_width(10);
  
  m_away_up_button.add( * manage( new Gtk::Image(Gtk::Stock::GO_UP, Gtk::ICON_SIZE_MENU) ) );
  m_away_up_button.signal_clicked().connect(  SigC::slot( *this, &SettingsDialog::away_message_up_cb ) );
  m_tooltip.set_tip(m_away_up_button, _("Move the selected message up in the list") );
  vbox2->pack_start( m_away_up_button, Gtk::PACK_SHRINK );

  m_away_remove_button.add( * manage( new Gtk::Image(Gtk::Stock::DELETE, Gtk::ICON_SIZE_MENU) ) );
  m_away_remove_button.signal_clicked().connect(  SigC::slot( *this, &SettingsDialog::away_message_remove_cb ) );
  m_tooltip.set_tip(m_away_remove_button, _("Delete the selected message") );
  vbox2->pack_start( m_away_remove_button, Gtk::PACK_SHRINK );

  m_away_down_button.add( * manage( new Gtk::Image(Gtk::Stock::GO_DOWN, Gtk::ICON_SIZE_MENU) ) );
  m_away_down_button.signal_clicked().connect(  SigC::slot( *this, &SettingsDialog::away_message_down_cb ) );
  m_tooltip.set_tip(m_away_down_button, _("Move the selected message down in the list") );
  vbox2->pack_start( m_away_down_button, Gtk::PACK_SHRINK );

  m_away_new_button.add( * manage( new Gtk::Image(Gtk::Stock::NEW, Gtk::ICON_SIZE_MENU) ) );
  m_away_new_button.signal_clicked().connect(  SigC::slot( *this, &SettingsDialog::away_message_new_cb ) );
  m_tooltip.set_tip(m_away_new_button, _("Create a new preset message") );
  vbox2->pack_end( m_away_new_button, Gtk::PACK_SHRINK );

  hbox->pack_start( *vbox2, Gtk::PACK_SHRINK );
  
  scr_win = manage( new Gtk::ScrolledWindow() );
  scr_win->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  scr_win->set_size_request(100, 100); // gtk - otherwise the text widget forces it to enlarge (!?)
  scr_win->set_shadow_type(Gtk::SHADOW_IN);

  m_away_up_button.set_sensitive(false);
  m_away_remove_button.set_sensitive(false);
  m_away_down_button.set_sensitive(false);

  m_away_textview.set_sensitive(false);
  m_away_textview.set_wrap_mode(Gtk::WRAP_WORD);

  m_away_textview.get_buffer()->signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_away_textview.get_buffer()->signal_changed().connect( SigC::slot( *this, &SettingsDialog::away_message_text_edit_cb ) );
  scr_win->add(m_away_textview);

  hbox->pack_start( * scr_win, Gtk::PACK_EXPAND_WIDGET );

  vbox2 = manage( new Gtk::VBox() );

  vbox2->pack_start( *hbox, Gtk::PACK_SHRINK );
  
  Gtk::Label * label = manage( new Gtk::Label( _("Note: When ickle idles and changes to away or N/A the first of these messages "
						 "will be used as the default auto response."), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP ) );
  label->set_line_wrap(true);
  label->set_justify(Gtk::JUSTIFY_FILL);

  vbox2->pack_start( *label, Gtk::PACK_EXPAND_WIDGET );

  frame->add( *vbox2 );

  vbox->pack_start( *frame, Gtk::PACK_SHRINK );

  add_page( _("Away Messages"), vbox, false );
}

void SettingsDialog::init_advanced_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();

  SectionFrame * frame = manage( new SectionFrame( _("Network") ) );

  Gtk::Table * table = manage( new Gtk::Table( 2, 3 ) );

  table->attach( * manage( new Gtk::Label( _("Server name"), 0.0, 0.5 ) ),
		 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );

  m_network_login_host.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_login_host.signal_changed().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_network_login_host,_("Specifies the name of the login server (IP or DNS name)"));
  table->attach( m_network_login_host, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );

  table->attach( * manage( new Gtk::Label( _("Server port"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );

  m_network_login_port.set_range(0, 0xffff);
  m_network_login_port.set_digits(0);
  m_network_login_port.set_increments(1, 10);
  m_network_login_port.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_login_port.signal_changed().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_network_login_port,_("Specifies the port of the login server. The default is 5190"));
  table->attach( m_network_login_port, 1, 2, 1, 2, Gtk::SHRINK, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);

  frame->add( * table );

  vbox->pack_start( *frame );

  add_page( _("Advanced"), vbox, true );

  // sub-pages
  init_advanced_security_page();
  init_advanced_smtp_page();
  init_advanced_logging_page();
}

void SettingsDialog::init_advanced_security_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();

  SectionFrame * frame = manage( new SectionFrame( _("Connections") ) );

  Gtk::Table * table = manage( new Gtk::Table( 4, 4 ) );

  m_network_in_dc.set_label(_("Allow incoming direct connections"));
  m_network_in_dc.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_in_dc.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_network_in_dc,_("Enable if you would like ickle to listen on a port to receive incoming direct connections"));
  table->attach( m_network_in_dc, 0, 4, 0, 1, Gtk::FILL, Gtk::FILL );

  m_network_out_dc.set_label(_("Allow outgoing direct connections"));
  m_network_out_dc.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_out_dc.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_network_out_dc,_("Enable if you would like ickle to attempt to make outgoing direct connections to contacts"));
  table->attach( m_network_out_dc, 0, 4, 1, 2, Gtk::FILL, Gtk::FILL );

  m_network_use_portrange.set_label(_("Specify port range for incoming connections"));
  m_network_use_portrange.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_use_portrange.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_network_use_portrange.signal_toggled().connect( SigC::bind<int>( SigC::slot(*this, &SettingsDialog::toggle_dc),0 ) );
  m_tooltip.set_tip (m_network_use_portrange,_("You can specify a port range to use when picking the local port to listen on. "
					       "Useful if you need to forward those fixed ports from your firewall onto your internal machine"));
  table->attach( m_network_use_portrange, 0, 4, 2, 3, Gtk::FILL, Gtk::FILL | Gtk::EXPAND );

  table->attach( * manage( new Gtk::Label( _("Local direct connection listening port from"), 0.0, 0.5 ) ),
		 0, 1, 3, 4, Gtk::FILL, Gtk::FILL );

  m_network_lower_bind_port.set_range(1024, 0xffff);
  m_network_lower_bind_port.set_digits(0);
  m_network_lower_bind_port.set_increments(1, 10);
  m_network_lower_bind_port.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_lower_bind_port.signal_changed().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_network_lower_bind_port.signal_value_changed().connect( SigC::bind<int>( SigC::slot(*this, &SettingsDialog::toggle_dc),1 ) );
  m_tooltip.set_tip (m_network_lower_bind_port,_("The lower value to use when picking a local port to bind to"));
  table->attach( m_network_lower_bind_port, 1, 2, 3, 4, Gtk::FILL, Gtk::FILL );

  table->attach( * manage( new Gtk::Label( _("to"), 0.0, 0.5 ) ),
		 2, 3, 3, 4, Gtk::FILL, Gtk::FILL );

  m_network_upper_bind_port.set_range(1024, 0xffff);
  m_network_upper_bind_port.set_digits(0);
  m_network_upper_bind_port.set_increments(1, 10);
  m_network_upper_bind_port.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_upper_bind_port.signal_changed().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_network_upper_bind_port.signal_value_changed().connect( SigC::bind<int>( SigC::slot(*this, &SettingsDialog::toggle_dc),2 ) );
  m_tooltip.set_tip (m_network_upper_bind_port,_("The upper value to use when picking a local port to bind to"));
  table->attach( m_network_upper_bind_port, 3, 4, 3, 4, Gtk::FILL, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);

  frame->add( * table );

  vbox->pack_start( *frame, Gtk::PACK_SHRINK );

  add_page( _("Security"), vbox, false );
}

void SettingsDialog::init_advanced_smtp_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  SectionFrame * frame = manage( new SectionFrame( _("SMTP") ) );

  Gtk::Table * table = manage( new Gtk::Table( 2, 3 ) );

  m_network_smtp.set_label(_("Use SMTP server"));
  m_network_smtp.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_smtp.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::toggle_smtp ) );
  m_network_smtp.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_network_smtp,
		     _("Use SMTP server. When sending SMS messages some mobile networks are supported through sending "
		       "messages through an email gateway by the provider. For these cases ickle will need an SMTP server to "
		       "send these emails through"));
  table->attach( m_network_smtp, 0, 2, 0, 1, Gtk::FILL, Gtk::FILL );

  table->attach( * manage( new Gtk::Label( _("SMTP server"), 0.0, 0.5 ) ),
		 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );

  m_network_smtp_host.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_smtp_host.signal_changed().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_network_smtp_host,_("Specifies the SMTP Server"));
  table->attach( m_network_smtp_host, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL );

  table->attach( * manage( new Gtk::Label( _("Server port"), 0.0, 0.5 ) ),
		 0, 1, 2, 3, Gtk::FILL, Gtk::FILL );

  m_network_smtp_port.set_range(0, 0xffff);
  m_network_smtp_port.set_digits(0);
  m_network_smtp_port.set_increments(1, 10);
  m_network_smtp_port.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_network_smtp_port.signal_changed().connect( SigC::slot( *this, &SettingsDialog::client_changed ) );
  m_tooltip.set_tip (m_network_smtp_port,_("Specifies the port of the SMTP server"));
  table->attach( m_network_smtp_port, 1, 2, 2, 3, Gtk::SHRINK, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);

  frame->add( * table );

  vbox->pack_start( *frame );

  add_page( _("SMTP"), vbox, false );
}
void SettingsDialog::init_advanced_logging_page()
{
  Gtk::VBox * vbox = new Gtk::VBox();
  SectionFrame * where = manage( new SectionFrame( _("Where") ) );
  Gtk::Table * table = manage( new Gtk::Table( 2, 3 ) );

  m_log_to_console.set_label(_("Console"));
  m_log_to_console.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_log_to_console,_("Choose to log messages to the console (ie. stdout)"));
  table->attach( m_log_to_console, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );

  m_log_to_file.set_label(_("File"));
  m_log_to_file.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_log_to_file.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::toggle_logfile ) );
  m_tooltip.set_tip (m_log_to_file,_("Choose to log messages to a log file"));
  table->attach( m_log_to_file, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );

  table->attach( * manage( new Gtk::Label( _("Log file"), 0.0, 0.5 ) ),
		 0, 1, 2, 3, Gtk::FILL, Gtk::FILL );

  m_logfile.signal_changed().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_logfile,_("Specify the location of the the log file to log to"));
  table->attach( m_logfile, 1, 2, 2, 3, Gtk::FILL, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);

  where->add( * table );

  vbox->pack_start( *where );

  SectionFrame * what = manage( new SectionFrame( _("What") ) );
  table = manage( new Gtk::Table( 2, 3 ) );

  m_log_directpacket.set_label(_("Direct packets"));
  m_log_directpacket.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_log_directpacket,_("Log packet data from direct connection packets"));
  table->attach( m_log_directpacket, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL );

  m_log_error.set_label(_("Errors"));
  m_log_error.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_log_error,_("Log error messages"));
  table->attach( m_log_error, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL );

  m_log_info.set_label(_("Infos"));
  m_log_info.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_log_info,_("Log informational messages"));
  table->attach( m_log_info, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL );

  m_log_packet.set_label(_("Packets"));
  m_log_packet.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_log_packet,_("Log client to server communication packets"));
  table->attach( m_log_packet, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL );

  m_log_warn.set_label(_("Warnings"));
  m_log_warn.signal_toggled().connect( SigC::slot( *this, &SettingsDialog::changed_cb ) );
  m_tooltip.set_tip (m_log_warn,_("Log warnings"));
  table->attach( m_log_warn, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL );

  table->set_spacings(5);
  table->set_border_width(10);

  what->add( * table );

  vbox->pack_start( *what );
  add_page( _("Logging"), vbox, false );
}

void SettingsDialog::load_pages()
{
  load_login_page();
  load_look_page();
  load_events_page();
  load_away_page();
  load_advanced_page();
  m_apply_button.set_sensitive(false);
}


void SettingsDialog::load_login_page()
{
  m_login_uin.set_value((double)g_settings.getValueUnsignedInt("uin") );
  m_login_pass.set_text( g_settings.getValueString("password") );
  m_auto_connect=ICQ2000::Status(g_settings.getValueUnsignedInt("autoconnect") );
  m_auto_reconnect.set_active(g_settings.getValueBool("autoreconnect") );
  m_reconnect_retries.set_value((double)g_settings.getValueUnsignedInt("reconnect_retries") );
  toggle_reconnect();

}

void SettingsDialog::load_look_page()
{
  load_look_message_page();
  load_look_contact_list_page();
  load_look_icons_page();
  load_look_charset_page();
  m_message_header_font.set_label(g_settings.getValueString("message_header_font") );
  m_message_text_font.set_label(g_settings.getValueString("message_text_font") );

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
  m_mouse_check_away_click.set_active(g_settings.getValueBool("mouse_check_away_click") );
  m_mouse_single_click.set_active(g_settings.getValueBool("mouse_single_click") );
}

void SettingsDialog::load_look_icons_page()
{
  m_icons_dir = g_settings.getValueString("icons_dir");
  
  std::string::size_type fin = m_icons_dir.size();
  if ( fin > 0 && m_icons_dir[fin-1] == '/' ) --fin;

  std::string::size_type n = std::string::npos;
  if ( fin > 0 ) n = m_icons_dir.rfind('/', fin-1);

  std::string name;
  if (n != std::string::npos)
    name = m_icons_dir.substr(n+1, fin-n-1);

  for (Gtk::TreeModel::Children::iterator curr = m_icons_reftreemodel->children().begin() ;
       curr != m_icons_reftreemodel->children().end() ;
       ++curr )
  {
    if ( (*curr)[m_icons_columns.name] == name )
      m_icons_treeview.get_selection()->select(curr);
  }
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
  // populate the list with entries
  int no_autoresponses = g_settings.getValueUnsignedInt("no_autoresponses");
  for (int i = 1; i <= no_autoresponses; ++i)
  {
    Gtk::TreeModel::Row row = * m_away_refliststore->append();

    std::string label = g_settings.getValueString( Utils::format_string( "autoresponse_%d_label", i ) );
    std::string text  = g_settings.getValueString( Utils::format_string( "autoresponse_%d_text", i ) );
    
    row[ m_away_columns.text ]  = text;
    row[ m_away_columns.label ] = label;

    // select the first entry
    if (i == 1)
      m_away_treeview.get_selection()->select( row );
  }
  
}

void SettingsDialog::load_events_page()
{
  m_event_message.set_text( g_settings.getValueString("event_message") );;
  m_event_sms.set_text( g_settings.getValueString("event_sms") );;
  m_event_system.set_text( g_settings.getValueString("event_system") );;
  m_event_url.set_text( g_settings.getValueString("event_url") );;
  m_event_user_online.set_text( g_settings.getValueString("event_user_online") );
  m_event_execute_all.set_active(g_settings.getValueBool("event_execute_all") );
  m_event_repetition_threshold.set_value(g_settings.getValueUnsignedInt("event_repetition_threshold") );
}

void SettingsDialog::load_advanced_page()
{
  m_network_login_host.set_text( g_settings.getValueString("network_login_host") );
  m_network_login_port.set_value((double) g_settings.getValueUnsignedInt("network_login_port") );
  m_network_override_port.set_active(g_settings.getValueBool("network_override_port") );

  load_advanced_security_page();
  load_advanced_smtp_page();
  load_advanced_logging_page();
}

void SettingsDialog::load_advanced_security_page()
{
  m_network_in_dc.set_active(g_settings.getValueBool("network_in_dc") );
  m_network_out_dc.set_active(g_settings.getValueBool("network_out_dc") );
  m_network_use_portrange.set_active(g_settings.getValueBool("network_use_portrange") );
  m_network_lower_bind_port.set_value((double) g_settings.getValueUnsignedInt("network_lower_bind_port") );
  m_network_upper_bind_port.set_value((double) g_settings.getValueUnsignedInt("network_upper_bind_port") );
  toggle_dc( 0 );
}



void SettingsDialog::load_advanced_smtp_page()
{
  m_network_smtp_host.set_text( g_settings.getValueString("network_smtp_host") );
  m_network_smtp_port.set_value((double) g_settings.getValueUnsignedInt("network_smtp_port") );
  m_network_smtp.set_active(g_settings.getValueBool("network_smtp") );
  toggle_smtp();
}

void SettingsDialog::load_advanced_logging_page()
{
 m_log_to_console.set_active(g_settings.getValueBool("log_to_console") );
 m_log_to_file.set_active(g_settings.getValueBool("log_to_file") );
 m_log_directpacket.set_active(g_settings.getValueBool("log_directpacket") );
 m_log_error.set_active(g_settings.getValueBool("log_error") );
 m_log_info.set_active(g_settings.getValueBool("log_info") );
 m_log_packet.set_active(g_settings.getValueBool("log_packet") );
 m_log_warn.set_active(g_settings.getValueBool("log_warn") );
 m_logfile.set_text(g_settings.getValueString("logfile") );
 toggle_logfile();
}


void SettingsDialog::save_pages()
{
  save_login_page();
  save_look_page();
  save_events_page();
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
  g_settings.setValue("message_header_font", m_message_header_font.get_label() );
  g_settings.setValue("message_text_font", m_message_text_font.get_label() );

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
  g_settings.setValue("mouse_check_away_click", m_mouse_check_away_click.get_active() );
  g_settings.setValue("mouse_single_click", m_mouse_single_click.get_active() );
}

void SettingsDialog::save_look_icons_page()
{
  g_settings.setValue("icons_dir", m_icons_dir );
}

void SettingsDialog::save_look_charset_page()
{
  g_settings.setValue( "encoding", m_lnf_charset.get_text() );
}


void SettingsDialog::save_events_page()
{
  g_settings.setValue( "event_message", m_event_message.get_text() );
  g_settings.setValue( "event_sms", m_event_sms.get_text() );
  g_settings.setValue( "event_system", m_event_system.get_text() );
  g_settings.setValue( "event_url", m_event_url.get_text() );
  g_settings.setValue( "event_user_online", m_event_user_online.get_text() );
  g_settings.setValue( "event_execute_all", m_event_execute_all.get_active() );
  g_settings.setValue( "event_repetition_threshold", (unsigned int) m_event_repetition_threshold.get_value() );
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
  unsigned short size = m_away_refliststore->children().size();
  
  g_settings.setValue("no_autoresponses", size );
  int i = 1;
  for ( Gtk::TreeModel::iterator curr = m_away_refliststore->children().begin() ;
	curr != m_away_refliststore->children().end() ;
	++curr, ++i )
  {
    Glib::ustring label = (*curr)[ m_away_columns.label ];
    Glib::ustring text  = (*curr)[ m_away_columns.text ];
    
    g_settings.setValue( Utils::format_string( "autoresponse_%d_label", i ), label );
    g_settings.setValue( Utils::format_string( "autoresponse_%d_text", i ),  text );
  }
}



//Saves the settings of the look page
void SettingsDialog::save_advanced_page()
{
  g_settings.setValue( "network_login_host", m_network_login_host.get_text() );
  g_settings.setValue( "network_login_port", (unsigned int) m_network_login_port.get_value() );
  g_settings.setValue( "network_override_port", m_network_override_port.get_active() );

  save_advanced_security_page();
  save_advanced_smtp_page();
  save_advanced_logging_page();
}

void SettingsDialog::save_advanced_security_page()
{
  g_settings.setValue( "network_in_dc", m_network_in_dc.get_active() );
  g_settings.setValue( "network_out_dc", m_network_out_dc.get_active() );
  g_settings.setValue( "network_use_portrange", m_network_use_portrange.get_active() );
  g_settings.setValue( "network_lower_bind_port", (unsigned int) m_network_lower_bind_port.get_value() );
  g_settings.setValue( "network_upper_bind_port", (unsigned int) m_network_upper_bind_port.get_value() );
}

void SettingsDialog::save_advanced_smtp_page()
{
  g_settings.setValue( "network_smtp_host", m_network_smtp_host.get_text() );
  g_settings.setValue( "network_smtp_port", (unsigned int) m_network_smtp_port.get_value() );
  g_settings.setValue( "network_smtp", m_network_smtp.get_active() );
}

void SettingsDialog::save_advanced_logging_page()
{
  g_settings.setValue( "log_to_console", m_log_to_console.get_active() );
  g_settings.setValue( "log_to_file", m_log_to_file.get_active() );
  g_settings.setValue( "log_directpacket", m_log_directpacket.get_active() );
  g_settings.setValue( "log_error", m_log_error.get_active() );
  g_settings.setValue( "log_info", m_log_info.get_active() );
  g_settings.setValue( "log_packet", m_log_packet.get_active() );
  g_settings.setValue( "log_warn", m_log_warn.get_active() );
  g_settings.setValue( "logfile", m_logfile.get_text() );
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

void SettingsDialog::icons_changed()
{
  // hmmm is this gtkmm 2.2 only ?
  Gtk::TreeModel::iterator iter = m_icons_treeview.get_selection()->get_selected();
  if ( iter )
  {
    m_icons_dir = ICONS_DIR + (*iter)[ m_icons_columns.name ] + "/";
    m_icons_changed = true;
    changed_cb();
  }
}
  

//ensures that SMTP setings can only be set if SMTP is activated
void SettingsDialog::toggle_smtp()
{
  if (m_network_smtp.get_active())
  {
    m_network_smtp_host.set_sensitive(true);
    m_network_smtp_port.set_sensitive(true);
  }
  else
  {
    m_network_smtp_host.set_sensitive(false);
    m_network_smtp_port.set_sensitive(false);
  }
}

//ensures that reconnect retries can only be set if reconnection is activated
void SettingsDialog::toggle_reconnect()
{
  if (m_auto_reconnect.get_active())
  {
    m_reconnect_retries.set_sensitive(true);
  }
  else
  {
     m_reconnect_retries.set_sensitive(false);
  }
}

//ensures that the log file can only be set if file logging is activated
void SettingsDialog::toggle_logfile()
{
   m_logfile.set_sensitive(m_log_to_file.get_active() );
}

//some settings of direct connection have side effects on others. Be aware of it
void SettingsDialog::toggle_dc(unsigned int what)
{
  switch (what)
  {
     case 0:	m_network_lower_bind_port.set_sensitive(m_network_use_portrange.get_active() );
     			m_network_upper_bind_port.set_sensitive(m_network_use_portrange.get_active() );
			break;
     case 1:       if (m_network_lower_bind_port.get_value() > m_network_upper_bind_port.get_value() )
     			  m_network_upper_bind_port.set_value( m_network_lower_bind_port.get_value () );
     			break;
     case 2:       if (m_network_lower_bind_port.get_value() > m_network_upper_bind_port.get_value() )
     			  m_network_lower_bind_port.set_value( m_network_upper_bind_port.get_value () );
     			break;
     default:  std::cerr << _("Error in SettingsDialog::toggle_dc. Unknown option ") << what << std::endl;
  }
}

//sends the settings to the Ickleclient and reconnects if necessary
void SettingsDialog::activate_changes()
{
  change_client.emit();
}

void SettingsDialog::away_message_up_cb()
{
  Gtk::TreeModel::iterator iter = m_away_treeview.get_selection()->get_selected();
  if (iter && iter != m_away_refliststore->children().begin())
  {
    Gtk::TreeModel::iterator iter2, iteri;
    
    // urgh.. why no operator-- :-(
    for ( iter2 = iteri = m_away_refliststore->children().begin(), ++iteri;
	  iteri != m_away_refliststore->children().end();
	  ++iteri, ++iter2 )
      if (iteri == iter) break;
    
    m_away_down_button.set_sensitive(true);
    if (iter2 == m_away_refliststore->children().begin())
      m_away_up_button.set_sensitive(false);

    /* this is some gtk bizarreness - doing the move iter to iter2
     * doesn't work for the case when iter2 == begin (it moves to end
     * instead), so instead we always do the swap by moving iter2 to
     * the space after iter, effectively swapping them in the same way
     * as required */
    m_away_refliststore->move( iter2, ++iter );
  }
}

void SettingsDialog::away_message_remove_cb()
{
  Gtk::TreeModel::iterator iter = m_away_treeview.get_selection()->get_selected();
  if (iter)
  {
    Gtk::TreeModel::Row row = *iter;
    m_away_textview.get_buffer()->set_text("");
    m_away_textview.set_sensitive(false);
    m_away_refliststore->erase(iter);
    // select the first
    if (m_away_refliststore->children().size())
      m_away_treeview.get_selection()->select( m_away_refliststore->children().begin() );
  }
}

void SettingsDialog::away_message_down_cb()
{
  Gtk::TreeModel::iterator iter = m_away_treeview.get_selection()->get_selected();
  if (iter)
  {
    Gtk::TreeModel::iterator iter2 = iter;
    ++iter2;
    if (iter2 != m_away_refliststore->children().end())
    {
      ++iter2;
      m_away_refliststore->move( iter, iter2 );

      m_away_up_button.set_sensitive(true);
      if (iter2 == m_away_refliststore->children().end())
	m_away_down_button.set_sensitive(false);
    }
  }
}

void SettingsDialog::away_message_new_cb()
{
  Gtk::TreeModel::Row row = * m_away_refliststore->append();
  row[ m_away_columns.text ]  = _("Enter your away response here.");
  row[ m_away_columns.label ] = _("New message");
  m_away_treeview.get_selection()->select( row );
}

void SettingsDialog::away_message_select_cb()
{
  Gtk::TreeModel::iterator iter = m_away_treeview.get_selection()->get_selected();
  if (iter)
  {
    Gtk::TreeModel::Row row = *iter;
    m_away_up_button.set_sensitive( iter != m_away_refliststore->children().begin() );
    m_away_remove_button.set_sensitive(true);
    m_away_down_button.set_sensitive( ++iter != m_away_refliststore->children().end() );
    m_away_textview.get_buffer()->set_text( row[ m_away_columns.text ] );
    m_away_textview.set_sensitive(true);
  }
  else
  {
    m_away_up_button.set_sensitive(false);
    m_away_remove_button.set_sensitive(false);
    m_away_down_button.set_sensitive(false);
    m_away_textview.get_buffer()->set_text("");
    m_away_textview.set_sensitive(false);
  }
}

void SettingsDialog::away_message_text_edit_cb()
{
  Gtk::TreeModel::iterator iter = m_away_treeview.get_selection()->get_selected();
  if (iter)
  {
    Gtk::TreeModel::Row row = *iter;
    row[ m_away_columns.text ] = m_away_textview.get_buffer()->get_text();
  }
}
  

void SettingsDialog::set_message_header_font_cb()
{
  Gtk::FontSelectionDialog fd;
  fd.set_title (_("Set header font") );
  fd.set_font_name(m_message_header_font.get_label() );
  if (fd.run() != Gtk::RESPONSE_OK )
    return;
  if (m_message_header_font.get_label() != fd.get_font_name() )
    m_fonts_changed=true;
  m_message_header_font.set_label(fd.get_font_name() );

}
void SettingsDialog::set_message_text_font_cb()
{
  Gtk::FontSelectionDialog fd;
  fd.set_title (_("Set text font") );
  fd.set_font_name(m_message_text_font.get_label() );
  if (fd.run() != Gtk::RESPONSE_OK )
    return;
  if (m_message_header_font.get_label() != fd.get_font_name() )
    m_fonts_changed=true;
  m_message_text_font.set_label(fd.get_font_name() );
}

void SettingsDialog::subs_cb()
{
	 PromptDialog pd (*this, Gtk::MESSAGE_INFO,_("Available substitutions:\n\n"
                  "%i\tExternal ip of the contact\n"
                  "%p\tExternal port of the contact\n"
                  "%e\tEmail address of the contact\n"
                  "%n\tName of the contact\n"
                  "%f\tFirst name of the contact\n"
                  "%l\tLast name of the contact\n"
                  "%a\tAlias of the contact\n"
                  "%u\tUIN of the contact\n"
                  "%c\tCellular phonenumber of the contact\n"
                  "%s\tStatus of the contact\n"
                  "%S\tDitto.\n"
                  "%t\tTimestamp for the event\n"
                  "%T\tTimestamp for the event with timezone\n"
                  "%r\tWhether the event is repeated\n"
                  "%m\tNumber of pending messages for the contact\n"
                  "%o\tWhen the contact went online\n") );
	pd.run();
}

