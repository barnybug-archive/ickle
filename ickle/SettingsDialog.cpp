/* $Id: SettingsDialog.cpp,v 1.55 2002-10-30 20:59:54 barnabygray Exp $
 *
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

#include "SettingsDialog.h"

#include "main.h"
#include <libicq2000/Client.h>
#include "Dir.h"
#include "Icons.h"
#include "PromptDialog.h"
#include "sstream_fix.h"

#include <gtk--/box.h>
#include <gtk--/table.h>
#include <gtk--/frame.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/optionmenu.h>
#include <gtk--/menu.h>
#include <gtk--/menushell.h>
#include <gtk--/adjustment.h>
#include <gtk--/fontselection.h>
#include <gtk--/fileselection.h>
#include <gtk--/menuitem.h>
#include <gtk--/listitem.h>
#include <gtk--/arrow.h>
#include <gtk--/buttonbox.h>

#include <algorithm>

using SigC::slot;
using SigC::bind;
using ICQ2000::Status;

using std::string;
using std::vector;
using std::list;
using std::ostringstream;
using std::swap;

SettingsDialog::SettingsDialog(Gtk::Window * parent)
  : Gtk::Dialog(),
    okay("OK"), cancel("Cancel"),
    subs_b("Substitutions legend..."),
    window_icons_check("Set window icons", 0),
    event_execute_all("Execute all events", 0),
    away_autoposition("Autoposition away messages dialog", 0),
    reconnect_checkbox("Auto Reconnect", 0),
    auto_return("Automatically return (from auto-away or auto-N/A)", 0),
    network_override_port("Override server redirect port with login port", 0),
    network_smtp("Use an SMTP server", 0),
    network_smtp_host_label("SMTP Host", 0),
    network_smtp_port_label("SMTP Port", 0),
    network_in_dc("Accept incoming direct connections", 0),
    network_out_dc("Make outgoing direct connections", 0),
    network_use_portrange("Specify port range for listening server", 0),
    network_lower_port_label("From:", 0),
    network_upper_port_label("To:", 0),
    message_autopopup("Autopopup on incoming message", 0),
    message_autoraise("Autoraise on incoming message", 0),
    message_autoclose("Autoclose after sending a message", 0),
    spell_check_no("None", 0),
    spell_check_ispell("Use ispell", 0),
    spell_check_aspell("Use aspell", 0),
    spell_check_lang(100),
    mouse_single_click("Single click opens Message Window", 0),
    mouse_check_away_click("Icon click checks Away Message", 0),
    status_cl_inv("Classic Invisibility", 0),
    popup_away_response("Popup auto response dialog on status change", 0),
    reconnect_label( "Retries", 0),
    history_shownr_label("Number of messages per history page", 0),
    log_info("Information", 0),
    log_warn("Warnings", 0),
    log_error("Errors", 0),
    log_packet("Server Packets", 0),
    log_directpacket("Direct Packets", 0),
    log_to_nowhere("The void", 0),
    log_to_console("Console", 0),
    log_to_file("File (~/.ickle/messages.log)", 0),
    log_to_consolefile("Selected to console, all to file", 0),
    away_remove_button("X"),
    away_response_list(1),
    away_response_label_entry(15),
    finished_okay(false)
{
  Gtk::VBox *vbox;
  Gtk::VBox *vbox2;

  set_title("ickle Settings");
  set_transient_for (*parent);
  set_modal(true);

  m_tooltips.set_tip(okay, "Accept the changes you have made");
  m_tooltips.set_tip(cancel, "Forget the changes you have made");
  okay.clicked.connect(slot(this,&SettingsDialog::okay_cb));
  cancel.clicked.connect(slot(this,&SettingsDialog::cancel_cb));

  notebook.set_tab_pos(GTK_POS_TOP);

  Gtk::HBox *hbox = get_action_area();
  hbox->set_border_width(0);
  Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  hbox->set_homogeneous(false);
  hbbox->pack_start(okay, false);
  hbbox->pack_start(cancel, false);

  hbox->pack_start( *hbbox );

  // ---------------- General tab -------------------------

  Gtk::Table *ftable = manage( new Gtk::Table( 2, 2, false ) );
  ftable->set_border_width(5);
  ftable->set_spacings(5);

  Gtk::Table *table = manage( new Gtk::Table( 2, 5, false ) );

  Gtk::Label *label;
  label = manage( new Gtk::Label( "UIN", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, 0);
  uin_entry.set_text(ICQ2000::Contact::UINtoString( g_settings.getValueUnsignedInt("uin") ));
  table->attach( uin_entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "Password", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, 0);
  password_entry.set_text( g_settings.getValueString("password") );
  password_entry.set_visibility(false);
  table->attach( password_entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "Auto Connect", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND, 0);

  Gtk::OptionMenu *autoconnect_om = manage( new Gtk::OptionMenu() );
  Gtk::Menu *m = manage( new Gtk::Menu() );
  {
    using namespace Gtk::Menu_Helpers;
    MenuList& ml = m->items();
    for (int n = ICQ2000::STATUS_ONLINE; n <= ICQ2000::STATUS_OFFLINE; n++)
      ml.push_back( MenuElem( ICQ2000::Status_text[n], bind( slot(this, &SettingsDialog::setStatus), Status(n) ) ) );
  }
  m_status = ICQ2000::Status(g_settings.getValueUnsignedInt("autoconnect"));
  m->set_active( m_status - ICQ2000::STATUS_ONLINE );
  autoconnect_om->set_menu(*m);

  table->attach( *autoconnect_om, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  unsigned char n_retr = g_settings.getValueUnsignedChar("reconnect_retries");

  Gtk::Adjustment *adj = manage( new Gtk::Adjustment( (n_retr == 0 ? 1 : n_retr), 1.0, 10.0 ) );
  reconnect_spinner = manage( new Gtk::SpinButton( *adj, 1.0, 0 ) );

  reconnect_checkbox.toggled.connect( slot( this, &SettingsDialog::reconnect_toggle_cb ) );
  reconnect_checkbox.set_active( n_retr > 0 );
  table->attach( reconnect_checkbox, 0, 2, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  table->attach( reconnect_label, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND, 0);

  table->attach( *reconnect_spinner, 1, 2, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  if ( n_retr == 0 ) {
    reconnect_label.set_sensitive(false);
    reconnect_spinner->set_sensitive(false);
  }

  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);

  Gtk::Frame *frame = manage( new Gtk::Frame("Login Details") );
  frame->add(*table);

  ftable->attach( *frame, 0, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  frame = manage( new Gtk::Frame("Translation") );
  vbox = manage( new Gtk::VBox() );
  trans_l.set_text( icqclient.getTranslationMapName() );
  m_tooltips.set_tip( trans_b, "Click to select a translation map from those installed.\n"
		      "This allows conversion from some of the bizarre character sets the windows client uses "
		      "to the unix standard. (ie. KOI-8 for Russian)");
  trans_b.set_border_width(10);
  trans_b.add(trans_l);
  trans_b.clicked.connect( slot( this, &SettingsDialog::trans_cb ) );
  vbox->pack_start(trans_b, false);
  frame->add(*vbox);

  ftable->attach( *frame, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  frame = manage( new Gtk::Frame("Icons") );
  vbox = manage( new Gtk::VBox() );
  vbox->set_border_width(5);

  icons_combo.set_value_in_list(true, false);

  string current = g_settings.getValueString("icons_dir");
  string current_name = "Default";
  list<string> icon_names;
  
  // Default icons (compiled in ones)
  icon_names.push_back( "Default" );
    
  // Icons in share/ickle/icons directory
  Dir dir;
  dir.setDirectoriesOnly(true);
  dir.list( ICONS_DIR + "*" );
  Dir::iterator iter = dir.begin();

  while (iter != dir.end()) {
    string name, filename = *iter;
    string::size_type pos = filename.rfind('/');
    if ( pos == string::npos) name = filename;
    else name = string(filename,pos+1);

    icon_names.push_back(name);
    if (current == filename+"/") current_name = name;
    ++iter;
  }

  icons_combo.set_popdown_strings(icon_names);
  icons_combo.get_entry()->set_editable(false);
  icons_combo.get_entry()->set_text(current_name);
  icons_combo.get_entry()->changed.connect( slot( this, &SettingsDialog::icons_cb ) );

  vbox->pack_start(icons_combo, true, true);

  window_icons_check.set_active(g_settings.getValueBool("window_status_icons"));
  m_tooltips.set_tip (window_icons_check, "Uncheck this if you don't want a user's status to be shown in "
                                          "the corresponding window's icon.\n"
                                          "This feature won't work correctly with some broken window managers. "
                                          "If you disable it, ickle must be restarted for the change to take effect.");

  vbox->pack_start(window_icons_check);
  frame->add(*vbox);

  ftable->attach( *frame, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  label = manage( new Gtk::Label( "General" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *ftable, *label )  );
  // ------------------------------------------------------------

  // ---------------- Events tab --------------------------

  table = manage( new Gtk::Table( 2, 9, false ) );
  
  label = manage( new Gtk::Label( "Below you can enter in commands to be executed when you receive "
                                  "an event. Leave them blank if you don't want anything to happen.", 0 ) );
  label->set_justify(GTK_JUSTIFY_FILL);
  label->set_line_wrap(true);
  table->attach( *label, 0, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK,
		 GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  subs_b.clicked.connect(slot(this,&SettingsDialog::subs_cb));
  table->attach( subs_b, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK );

  label = manage( new Gtk::Label( "User Online Event", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND, 0);
  event_user_online_entry.set_text( g_settings.getValueString("event_user_online") );
  table->attach( event_user_online_entry, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "Message Event", 0 ) );
  table->attach( *label, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND, 0);
  event_message_entry.set_text( g_settings.getValueString("event_message") );
  table->attach( event_message_entry, 1, 2, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "URL Event", 0 ) );
  table->attach( *label, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND, 0);
  event_url_entry.set_text( g_settings.getValueString("event_url") );
  table->attach( event_url_entry, 1, 2, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "SMS Event", 0 ) );
  table->attach( *label, 0, 1, 5, 6, GTK_FILL | GTK_EXPAND, 0);
  event_sms_entry.set_text( g_settings.getValueString("event_sms") );
  table->attach( event_sms_entry, 1, 2, 5, 6, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "System Event", 0 ) );
  table->attach( *label, 0, 1, 6, 7, GTK_FILL | GTK_EXPAND, 0);
  event_system_entry.set_text( g_settings.getValueString("event_system") );
  table->attach( event_system_entry, 1, 2, 6, 7, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "Event repetition threshold", 0 ) );
  table->attach( *label, 0, 1, 7, 8, GTK_FILL | GTK_EXPAND, 0);
  hbox = manage( new Gtk::HBox(false, 5) );  
  adj = manage( new Gtk::Adjustment( g_settings.getValueUnsignedInt("event_repetition_threshold"), 0.0, 65535.0 ) );
  event_repetition_spinner = manage ( new Gtk::SpinButton (*adj, 10.0, 0) );
  hbox->pack_start ( *event_repetition_spinner );
  label = manage ( new Gtk::Label( "milliseconds", 0) );
  hbox->pack_end ( *label, false, false );
  table->attach( *hbox, 1, 2, 7, 8, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  event_execute_all.set_active( g_settings.getValueBool("event_execute_all") );
  table->attach( event_execute_all, 0, 2, 8, 9, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  m_tooltips.set_tip( *event_repetition_spinner, "Filters out events occuring in quick succession" );
  m_tooltips.set_tip( event_execute_all, "Always executes the commands above, even if they're below the threshold.\n"
                                         "You can then use the %r substitution to check for repeated events." );

  label = manage( new Gtk::Label( "Events" ) );
  table->set_row_spacings(5);
  table->set_col_spacings(10);
  table->set_border_width(10);

  frame = manage( new Gtk::Frame("Events") );
  frame->set_border_width(5);
  frame->add(*table);

  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *frame, *label )  );

  // ---------------------------------------------------------

  // ------------------ Message Box --------------------------

  vbox2 = manage( new Gtk::VBox() );

  table = manage( new Gtk::Table( 2, 4, false ) );
  
  message_autopopup.set_active( g_settings.getValueBool("message_autopopup") );
  message_autoraise.set_active( g_settings.getValueBool("message_autoraise") );
  message_autoclose.set_active( g_settings.getValueBool("message_autoclose") );
  table->attach( message_autopopup, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, 0);
  table->attach( message_autoraise, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, 0);
  table->attach( message_autoclose, 0, 2, 1, 2, GTK_FILL | GTK_EXPAND, 0);

  n_retr = g_settings.getValueUnsignedChar("history_shownr");
  Gtk::Adjustment *history_shownr_adj = manage( new Gtk::Adjustment( n_retr, 1.0, 255.0 ) );
  history_shownr_spinner = manage( new Gtk::SpinButton( *history_shownr_adj, 1.0, 0 ) );
  table->attach( history_shownr_label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND, 0 );
  table->attach( *history_shownr_spinner, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND, 0 );
  
  hbox = manage( new Gtk::HBox(true, 5) );

  message_header_font = g_settings.getValueString("message_header_font");
  message_text_font = g_settings.getValueString("message_text_font");

  Gtk::Button *button;
  button = manage( new Gtk::Button("Header Font...") );
  button->clicked.connect( bind( slot( this, &SettingsDialog::fontsel_cb ), 0 ) );
  hbox->pack_start( *button, false );

  button = manage( new Gtk::Button("Text Font...") );
  button->clicked.connect( bind( slot( this, &SettingsDialog::fontsel_cb ), 1 ) );
  hbox->pack_end( *button, false );

  table->attach( *hbox, 0, 2, 3, 4, GTK_FILL | GTK_EXPAND, 0 );

  table->set_row_spacings(5);
  table->set_col_spacings(5);
  table->set_border_width(5);

  frame = manage( new Gtk::Frame("Message Box") );
  frame->add(*table);
  vbox2->pack_start( *frame );

  vbox = manage( new Gtk::VBox() );
  vbox->set_border_width(5);
  vbox->set_spacing(5);

  hbox = manage( new Gtk::HBox() );

  spell_check_ispell.set_group( spell_check_no.group() );
  spell_check_aspell.set_group( spell_check_no.group() );
  
  if ( g_settings.getValueBool("spell_check") ) {
    if ( g_settings.getValueBool("spell_check_aspell") )
      spell_check_aspell.set_active(true);
    else
      spell_check_ispell.set_active(true);
  }
  else
    spell_check_no.set_active( true );
  
  hbox->pack_start(spell_check_no);
  hbox->pack_start(spell_check_ispell);
  hbox->pack_start(spell_check_aspell);
  
  vbox->pack_start( *hbox );
  
  hbox = manage( new Gtk::HBox() );
  label = manage( new Gtk::Label( "Language (e.g. english, german or french)\nLeave blank for default", 0, 0 ) );
  label->set_justify(GTK_JUSTIFY_LEFT);
  hbox->pack_start(*label);
  spell_check_lang.set_text(g_settings.getValueString("spell_check_lang"));
  hbox->pack_start(spell_check_lang);
  
  vbox->pack_start( *hbox );

  frame = manage( new Gtk::Frame("Spell checking") );
  frame->add(*vbox);
  vbox2->pack_start( *frame );

  vbox2->set_border_width(5);
  vbox2->set_spacing(5);

  label = manage( new Gtk::Label( "Message Box" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *vbox2, *label )  );

  // ---------------------------------------------------------

  // ------------------ Away Status ------------------------

  table = manage( new Gtk::Table( 2, 6, false ) );
  
  away_autoposition.set_active( g_settings.getValueBool("away_autoposition") );
  table->attach( away_autoposition, 0, 2, 0, 1, GTK_FILL | GTK_EXPAND, 0);

  m_tooltips.set_tip( status_cl_inv, "If checked, invisibility will behave like any other status, "
		      "the way some people are used to it working.");
  status_cl_inv.set_active( g_settings.getValueBool("status_classic_invisibility") );
  table->attach( status_cl_inv, 0, 2, 1, 2, GTK_FILL | GTK_EXPAND, 0);

  popup_away_response.set_active( g_settings.getValueBool("set_away_response_dialog") );
  table->attach( popup_away_response, 0, 2, 2, 3, GTK_FILL | GTK_EXPAND, 0);

  label = manage( new Gtk::Label( "Auto-away (minutes)", 0 ) );
  unsigned short time = g_settings.getValueUnsignedShort( "auto_away" );
  adj = manage( new Gtk::Adjustment( time, 0.0, 65535.0 ) );
  autoaway_spinner = manage( new Gtk::SpinButton( *adj, 1.0, 0 ) );
  autoaway_spinner->set_editable(false);
  autoaway_spinner->changed.connect( bind( slot( this, &SettingsDialog::spinner_changed ), autoaway_spinner ) );
  table->attach( *label, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);
  table->attach( *autoaway_spinner, 1, 2, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);
  autoaway_spinner->changed ();

  label = manage( new Gtk::Label( "Auto-N/A (minutes)", 0 ) );
  time = g_settings.getValueUnsignedShort( "auto_na" );
  adj = manage( new Gtk::Adjustment( time, 0.0, 65535.0 ) );
  autona_spinner = manage( new Gtk::SpinButton( *adj, 1.0, 0 ) );
  autona_spinner->set_editable(false);
  autona_spinner->changed.connect( bind( slot( this, &SettingsDialog::spinner_changed ), autona_spinner ) );
  table->attach( *label, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);
  table->attach( *autona_spinner, 1, 2, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);
  autona_spinner->changed ();

  auto_return.set_active( g_settings.getValueBool("auto_return") );
  table->attach( auto_return, 0, 2, 5, 6, GTK_FILL | GTK_EXPAND, 0);

  table->set_row_spacings(5);
  table->set_col_spacings(5);
  table->set_border_width(5);

  vbox = manage(new Gtk::VBox());
  vbox->pack_start(*table);

  frame = manage( new Gtk::Frame("Away Status") );
  frame->set_border_width(5);
  frame->add(*vbox);
  vbox->set_border_width(5);
  
  label = manage( new Gtk::Label( "Away Status" ) );

  /* Edit away-messages */
  away_response_msg.set_editable(true);
  away_response_msg.set_usize(100,50);
  {
    using namespace Gtk::CList_Helpers;

    RowList& il = away_response_list.rows();

    unsigned int no_autoresponses = g_settings.getValueUnsignedInt("no_autoresponses");
    for (unsigned int i = 1; i <= no_autoresponses; i++) {
      ostringstream fetch_str, fetch_str2;
      fetch_str << "autoresponse_" << i << "_label";
      away_response_label_list.push_back( g_settings.getValueString(fetch_str.str()) );

      fetch_str2 << "autoresponse_" << i << "_text";
      away_response_msg_list.push_back( g_settings.getValueString(fetch_str2.str()) );

      vector<string> a;
      a.push_back(g_settings.getValueString(fetch_str.str()));
      RowList::iterator ri = il.insert( il.end(), a );
    }

    vector<string> a;
    a.push_back( "[new]" );
    RowList::iterator ri = il.insert( il.end(), a );
  }

  hbox = manage( new Gtk::HBox(false) );

  hbox->pack_start( away_response_list );

  vbox2 = manage( new Gtk::VBox() );

  away_up_button.add( * manage( new Gtk::Arrow(GTK_ARROW_UP, GTK_SHADOW_ETCHED_IN) ) );
  away_up_button.clicked.connect( slot(this, &SettingsDialog::away_up_button_cb ) );
  away_down_button.add( * manage( new Gtk::Arrow(GTK_ARROW_DOWN, GTK_SHADOW_ETCHED_IN) ) );
  away_down_button.clicked.connect( slot(this, &SettingsDialog::away_down_button_cb ) );
  
  m_tooltips.set_tip( away_up_button, "Move selected away message upwards in the list");
  m_tooltips.set_tip( away_down_button, "Move selected away message downwards in the list");
  m_tooltips.set_tip( away_remove_button, "Remove the selected away message from the list");

  vbox2->pack_start( away_up_button, true );

  vbox2->pack_start( away_remove_button, true );
  away_remove_button.clicked.connect( slot(this, &SettingsDialog::away_remove_button_cb ) );

  vbox2->pack_start( away_down_button, true );
  vbox2->set_spacing(5);

  hbox->pack_start( *vbox2, false, false, 5 );

  away_response_list.set_selection_mode(GTK_SELECTION_BROWSE);
  away_response_list.select_row.connect( slot( this, &SettingsDialog::away_response_list_select_row_cb ) );
  away_current_item_number = away_response_msg_list.size();
  away_response_list.row(0).select();

  vbox2 = manage( new Gtk::VBox() );

  away_response_label_edit_dead = false;
  away_response_label_entry.changed.connect( slot( this, &SettingsDialog::away_response_label_edit ) );
  away_response_label_entry.set_max_length(255);

  vbox2->pack_start( away_response_label_entry );
  vbox2->pack_start( away_response_msg );

  hbox->pack_end(*vbox2);

  vbox->pack_start(*hbox);
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *frame, *label )  );

  // ---------------------------------------------------------

  // ------------------ Logging ------------------------------

  Gtk::Table *ttable;
  ttable = manage( new Gtk::Table( 1, 2, false ) );

  table = manage( new Gtk::Table( 1, 5, false ) );

  log_info.set_active( g_settings.getValueBool("log_info") );
  table->attach( log_info, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, 0 );
  log_warn.set_active( g_settings.getValueBool("log_warn") );
  table->attach( log_warn, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, 0 );
  log_error.set_active( g_settings.getValueBool("log_error") );
  table->attach( log_error, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND, 0 );
  log_packet.set_active( g_settings.getValueBool("log_packet") );
  table->attach( log_packet, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND, 0 );
  log_directpacket.set_active( g_settings.getValueBool("log_directpacket") );
  table->attach( log_directpacket, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND, 0 );

  table->set_row_spacings(2);
  table->set_col_spacings(5);
  table->set_border_width(10);

  ttable->attach( *table, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0 );

  table = manage( new Gtk::Table( 2, 4, false ) );

  label = manage( new Gtk::Label( "Log to" ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, 0 );

  log_to_console.set_group( log_to_nowhere.group() );
  log_to_file.set_group( log_to_nowhere.group() );
  log_to_consolefile.set_group( log_to_nowhere.group() );

  m_tooltips.set_tip( log_to_nowhere, "Log messages are ignored");
  m_tooltips.set_tip( log_to_console, "Log messages are sent to the console ickle was started on");
  m_tooltips.set_tip( log_to_file, "Log messages are written to the log file.\n"
		      "Warning: this can get quite large!");
  m_tooltips.set_tip( log_to_consolefile, "Log messages are sent to console and written to file");
  
  bool log_to_console_b = g_settings.getValueBool("log_to_console");
  bool log_to_file_b = g_settings.getValueBool("log_to_file");
  if (!log_to_console_b)
    if (!log_to_file_b) log_to_nowhere.set_active(true);
    else log_to_file.set_active(true);
  else
    if (!log_to_file_b) log_to_console.set_active(true);
    else log_to_consolefile.set_active(true);

  table->attach( log_to_nowhere, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0 );
  table->attach( log_to_console, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0 );
  table->attach( log_to_file, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0 );
  table->attach( log_to_consolefile, 1, 2, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0 );
  
  table->set_row_spacings(3);
  table->set_col_spacings(5);
  table->set_border_width(10);

  ttable->attach( *table, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0 );

  frame = manage( new Gtk::Frame("Network Log") );
  frame->set_border_width(5);
  frame->add(*ttable);

  label = manage( new Gtk::Label( "Logging" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *frame, *label )  );

  // ---------------------------------------------------------


  // ------------------ Network ------------------------------

  table = manage( new Gtk::Table( 2, 10, false ) );

  label = manage( new Gtk::Label( "Login Host", 0 ) );
  table->attach( *label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, 0);

  network_host.set_text( g_settings.getValueString("network_login_host") );
  table->attach( network_host, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);
  
  label = manage( new Gtk::Label( "Login Port", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, 0);

  unsigned short port = g_settings.getValueUnsignedShort( "network_login_port" );
  adj = manage( new Gtk::Adjustment( port, 1.0, 65535.0 ) );
  network_port = manage( new Gtk::SpinButton( *adj, 1.0, 0 ) );
  table->attach( *network_port, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);
  
  network_override_port.set_active( g_settings.getValueBool("network_override_port") );
  table->attach( network_override_port, 0, 2, 2, 3, GTK_FILL | GTK_EXPAND, 0);

  m_tooltips.set_tip( network_in_dc, "Determine whether to start a listening server on a local port, "
		      "to accept incoming direct connections");
  m_tooltips.set_tip( network_out_dc, "Determine whether to attempt to make outgoing connections when messaging people.\n"
		      "Failing this, ickle will fallback on sending the message through the server");

  network_in_dc.set_active( g_settings.getValueBool("network_in_dc") );
  table->attach( network_in_dc, 0, 2, 3, 4, GTK_FILL | GTK_EXPAND, 0);

  network_out_dc.set_active( g_settings.getValueBool("network_out_dc") );
  table->attach( network_out_dc, 0, 2, 4, 5, GTK_FILL | GTK_EXPAND, 0);

  network_use_portrange.set_active( g_settings.getValueBool( "network_use_portrange" ) );
  network_use_portrange.toggled.connect( slot( this, &SettingsDialog::network_port_range_toggle_cb ) );
  table->attach( network_use_portrange, 0, 2, 5, 6, GTK_FILL | GTK_EXPAND, 0);
  
  m_tooltips.set_tip( network_use_portrange, "If you are behind a firewall and still wish to be able to "
		      "receive direct connections then you may get ickle to only use a certain port range "
		      "for its listening server. You can then configure your firewall to let through connections "
		      "to this restricted range. A sensible range is 9000-9010.");

  hbox = manage( new Gtk::HBox() );

  hbox->pack_start( network_lower_port_label );

  unsigned lower, upper;
  lower = g_settings.getValueUnsignedShort( "network_lower_bind_port" );
  upper = g_settings.getValueUnsignedShort( "network_upper_bind_port" );

  adj = manage( new Gtk::Adjustment( lower, 1.0, upper ) );
  network_lower_port_spinner = manage( new Gtk::SpinButton( *adj, 1.0, 0 ) );
  hbox->pack_start( *network_lower_port_spinner );

  hbox->pack_start( network_upper_port_label );

  adj = manage( new Gtk::Adjustment( upper, lower, 65535.0 ) );
  network_upper_port_spinner = manage( new Gtk::SpinButton( *adj, 1.0, 0 ) );
  hbox->pack_start( *network_upper_port_spinner );

  network_lower_port_spinner->get_adjustment()->value_changed.connect( slot( this, &SettingsDialog::network_port_range_lower_cb ) );
  network_upper_port_spinner->get_adjustment()->value_changed.connect( slot( this, &SettingsDialog::network_port_range_upper_cb ) );

  network_port_range_update();
  // setup enabled or disabled for widget initially

  table->attach( *hbox, 0, 2, 6, 7, GTK_FILL | GTK_EXPAND, 0);

  m_tooltips.set_tip( network_smtp, "Some Mobile providers provide SMS support for their phones "
		      "by using an SMTP (email) gateway.\n"
		      "If you would like to send messages to someone using one of these providers "
		      "then supply your SMTP details here.\n"
		      "See http://web.icq.com/sms/ for more details.");

  network_smtp.set_active( g_settings.getValueBool("network_smtp") );
  network_smtp.toggled.connect( slot( this, &SettingsDialog::network_smtp_toggle_cb ) );
  table->attach( network_smtp, 0, 2, 7, 8, GTK_FILL | GTK_EXPAND, 0);

  table->attach( network_smtp_host_label, 0, 1, 8, 9, GTK_FILL | GTK_EXPAND, 0);

  network_smtp_host.set_text( g_settings.getValueString("network_smtp_host") );
  table->attach( network_smtp_host, 1, 2, 8, 9, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);
  
  table->attach( network_smtp_port_label, 0, 1, 9, 10, GTK_FILL | GTK_EXPAND, 0);

  port = g_settings.getValueUnsignedShort( "network_smtp_port" );
  adj = manage( new Gtk::Adjustment( port, 1.0, 65535.0 ) );
  network_smtp_port = manage( new Gtk::SpinButton( *adj, 1.0, 0 ) );
  table->attach( *network_smtp_port, 1, 2, 9, 10, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  network_smtp_update();
  // setup enabled or disabled for widget initially
  
  table->set_row_spacings(5);
  table->set_col_spacings(5);
  table->set_border_width(10);

  frame = manage( new Gtk::Frame("Network") );
  frame->set_border_width(5);
  frame->add(*table);

  label = manage( new Gtk::Label( "Network" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *frame, *label )  );

  // ---------------------------------------------------------


  // ------------------ Contact List ------------------------------

  table = manage( new Gtk::Table( 2, 1, false ) );
  
  mouse_single_click.set_active( g_settings.getValueBool("mouse_single_click"));
  table->attach( mouse_single_click, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, 0);

  mouse_check_away_click.set_active( g_settings.getValueBool("mouse_check_away_click"));
  table->attach( mouse_check_away_click, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, 0);

  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);

  frame = manage( new Gtk::Frame("Contact List") );
  frame->set_border_width(5);
  frame->add(*table);

  label = manage( new Gtk::Label( "Contact List" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *frame, *label )  );
  
  // ---------------------------------------------------------


  // ------------------ Applet ------------------------------

#ifdef GNOME_ICKLE
  table = manage( new Gtk::Table( 2, 1, false ) );

  hidegui_onstart = manage( new Gtk::CheckButton( "Hide main window upon start" ) );
  hidegui_onstart->set_active( g_settings.getValueBool("hidegui_onstart") );

  table->attach( *hidegui_onstart, 0, 2, 0, 1, GTK_FILL | GTK_SHRINK, 0 );

  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);

  frame = manage( new Gtk::Frame("Applet Options") );
  frame->set_border_width(5);
  frame->add(*table);

  label = manage( new Gtk::Label( "Applet" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *frame, *label )  );

#endif
  
  // ---------------------------------------------------------

  vbox = get_vbox();
  vbox->pack_start( notebook, true, true );
  vbox->set_border_width (10);
  vbox->set_spacing (10);

  show_all();
}

void SettingsDialog::setStatus(Status s) {
  m_status = s;
}

bool SettingsDialog::run() {
  Gtk::Main::run();
  return finished_okay;
}

void SettingsDialog::updateSettings() {
  // ------------ Login details tab ----------------
  g_settings.setValue("uin", ICQ2000::Contact::StringtoUIN(uin_entry.get_text()));
  g_settings.setValue("password", password_entry.get_text());
  g_settings.setValue("autoconnect", m_status );
  
  if ( reconnect_checkbox.get_active() ) {
    g_settings.setValue( "reconnect_retries", (unsigned char)reconnect_spinner->get_value_as_int() );
  } else {
    g_settings.setValue( "reconnect_retries", (unsigned char)0 );
  }

  g_settings.setValue("translation_map", icqclient.getTranslationMapFileName() );
  g_settings.setValue("icons_dir", getIconsFilename());
  bool b = g_settings.getValueBool("window_status_icons");
  g_settings.setValue("window_status_icons", window_icons_check.get_active());
  if (window_icons_check.get_active() != b) g_icons.icons_changed();

  // ------------ Events tab -----------------------
  g_settings.setValue("event_user_online", event_user_online_entry.get_text());
  g_settings.setValue("event_message", event_message_entry.get_text());
  g_settings.setValue("event_url", event_url_entry.get_text());
  g_settings.setValue("event_sms", event_sms_entry.get_text());
  g_settings.setValue("event_system", event_system_entry.get_text());
  g_settings.setValue("event_repetition_threshold", (unsigned int)event_repetition_spinner->get_value_as_int());
  g_settings.setValue("event_execute_all", event_execute_all.get_active() );

  // ------------ Message Box tab ------------------
  g_settings.setValue("message_autopopup", message_autopopup.get_active() );
  g_settings.setValue("message_autoraise", message_autoraise.get_active() );
  g_settings.setValue("message_autoclose", message_autoclose.get_active() );
  g_settings.setValue("history_shownr", (unsigned char)history_shownr_spinner->get_value_as_int() );
  g_settings.setValue("message_header_font", message_header_font );
  g_settings.setValue("message_text_font", message_text_font );

  g_settings.setValue("spell_check", !spell_check_no.get_active() );
  g_settings.setValue("spell_check_aspell", spell_check_aspell.get_active() );
  g_settings.setValue("spell_check_lang", spell_check_lang.get_text());
  
  // ------------ Away Status tab ----------------
  g_settings.setValue("away_autoposition", away_autoposition.get_active() );
  g_settings.setValue("status_classic_invisibility", status_cl_inv.get_active() );
  g_settings.setValue("set_away_response_dialog", popup_away_response.get_active() );
  g_settings.setValue("auto_away", (unsigned short)autoaway_spinner->get_value_as_int());
  g_settings.setValue("auto_na", (unsigned short)autona_spinner->get_value_as_int());
  g_settings.setValue("auto_return", auto_return.get_active() );

  /* Predefined away-messages */

  // store current
  if (away_current_item_number < away_response_msg_list.size()) {
    // save old
    away_response_msg_list[away_current_item_number] = away_response_msg.get_chars(0, -1);
    away_response_label_list[away_current_item_number] = away_response_label_entry.get_text();
  }

  g_settings.setValue("no_autoresponses", (unsigned short)away_response_msg_list.size());
  for (unsigned int i = 0; i < away_response_msg_list.size(); i++) {
    ostringstream fetch_str;
    fetch_str << "autoresponse_" << i + 1 << "_label";
    g_settings.setValue(fetch_str.str(), away_response_label_list[i]);
    ostringstream fetch_str2;
    fetch_str2 << "autoresponse_" << i + 1 << "_text";
    g_settings.setValue(fetch_str2.str(), away_response_msg_list[i]);
  }

  // ------------ Logging tab ----------------------
  g_settings.setValue("log_info", log_info.get_active() );
  g_settings.setValue("log_warn", log_warn.get_active() );
  g_settings.setValue("log_error", log_error.get_active() );
  g_settings.setValue("log_packet", log_packet.get_active() );
  g_settings.setValue("log_directpacket", log_directpacket.get_active() );

  bool log_to_console_b = log_to_console.get_active() || log_to_consolefile.get_active();
  bool log_to_file_b = log_to_file.get_active() || log_to_consolefile.get_active();
  g_settings.setValue("log_to_console", log_to_console_b);
  g_settings.setValue("log_to_file", log_to_file_b);

  // ------------ Network tab ----------------------
  g_settings.setValue("network_login_host", network_host.get_text());
  g_settings.setValue("network_login_port", (unsigned short)network_port->get_value_as_int());
  g_settings.setValue("network_override_port", network_override_port.get_active());
  g_settings.setValue("network_in_dc", network_in_dc.get_active());
  g_settings.setValue("network_out_dc", network_out_dc.get_active());

  bool use_portrange = network_use_portrange.get_active();
  g_settings.setValue("network_use_portrange", use_portrange);
  g_settings.setValue("network_lower_bind_port", (unsigned short)network_lower_port_spinner->get_value_as_int());
  g_settings.setValue("network_upper_bind_port", (unsigned short)network_upper_port_spinner->get_value_as_int());

  g_settings.setValue("network_smtp", network_smtp.get_active());
  g_settings.setValue("network_smtp_host", network_smtp_host.get_text() );
  g_settings.setValue("network_smtp_port", (unsigned short)network_smtp_port->get_value_as_int());

  // synchronise changes with library
  icqclient.setLoginServerHost( g_settings.getValueString("network_login_host") );
  icqclient.setLoginServerPort( g_settings.getValueUnsignedShort("network_login_port") );
  if (g_settings.getValueBool("network_override_port")) {
    icqclient.setBOSServerOverridePort(true);
    icqclient.setBOSServerPort( g_settings.getValueUnsignedShort("network_login_port") );
  }
  icqclient.setAcceptInDC( g_settings.getValueBool("network_in_dc") );
  icqclient.setUseOutDC( g_settings.getValueBool("network_out_dc") );

  icqclient.setUsePortRange(use_portrange);
  if (use_portrange) {
    // use a port range
    icqclient.setPortRangeLowerBound( g_settings.getValueUnsignedShort("network_lower_bind_port") );
    icqclient.setPortRangeUpperBound( g_settings.getValueUnsignedShort("network_upper_bind_port") );
  }
  
  if (g_settings.getValueBool("network_smtp")) {
    // enable SMTP
    icqclient.setSMTPServerHost( g_settings.getValueString("network_smtp_host") );
    icqclient.setSMTPServerPort( g_settings.getValueUnsignedShort("network_smtp_port") );
  } else {
    // disable SMTP
    icqclient.setSMTPServerHost( "" );
    icqclient.setSMTPServerPort( 0 );
  }
  
  // ------------ Contact List tab ----------------------
  g_settings.setValue("mouse_single_click", mouse_single_click.get_active());
  g_settings.setValue("mouse_check_away_click", mouse_check_away_click.get_active());

  // ------------ Applet tab ----------------------
#ifdef GNOME_ICKLE
  g_settings.setValue("hidegui_onstart", hidegui_onstart->get_active() );
#endif  
}

unsigned int SettingsDialog::getUIN() const {
  return ICQ2000::Contact::StringtoUIN(uin_entry.get_text());
}

string SettingsDialog::getPassword() const {
  return password_entry.get_text();
}

void SettingsDialog::reconnect_toggle_cb() {
  if ( reconnect_checkbox.get_active() ) {
    reconnect_label.set_sensitive(true);
    reconnect_spinner->set_sensitive(true);
  } else {
    reconnect_label.set_sensitive(false);
    reconnect_spinner->set_sensitive(false);
  }
}

void SettingsDialog::network_smtp_toggle_cb()
{
  network_smtp_update();
}

void SettingsDialog::network_smtp_update()
{
  bool b = network_smtp.get_active();
  network_smtp_host_label.set_sensitive(b);
  network_smtp_host.set_sensitive(b);
  network_smtp_port_label.set_sensitive(b);
  network_smtp_port->set_sensitive(b);
}

void SettingsDialog::network_port_range_toggle_cb()
{
  network_port_range_update();
}

void SettingsDialog::network_port_range_lower_cb()
{
  Gtk::Adjustment *adj = network_lower_port_spinner->get_adjustment();
  network_upper_port_spinner->get_adjustment()->set_lower( adj->get_value() );
}

void SettingsDialog::network_port_range_upper_cb()
{
  Gtk::Adjustment *adj = network_upper_port_spinner->get_adjustment();
  network_lower_port_spinner->get_adjustment()->set_upper( adj->get_value() );
}

void SettingsDialog::network_port_range_update()
{
  bool b = network_use_portrange.get_active();
  network_lower_port_label.set_sensitive(b);  
  network_lower_port_spinner->set_sensitive(b);  
  network_upper_port_label.set_sensitive(b);  
  network_upper_port_spinner->set_sensitive(b);  
}

void SettingsDialog::okay_cb() {
  finished_okay = true;
  Gtk::Main::quit();
}

void SettingsDialog::cancel_cb() {
  finished_okay = false;

  // restore icons
  g_icons.setIcons( g_settings.getValueString("icons_dir") );
  
  Gtk::Main::quit();
}

void SettingsDialog::subs_cb()
{
  PromptDialog pd(this, PromptDialog::PROMPT_INFO,
                  "Available substitutions:\n\n"
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
		  "%o\tWhen the contact went online\n"
                  );
  pd.run();
}

void SettingsDialog::trans_cb() {
  Gtk::FileSelection filesel("Find the translation map");
  filesel.destroy.connect( Gtk::Main::quit.slot() );
  filesel.get_ok_button()->clicked.connect( bind( slot( this, &SettingsDialog::trans_ok_cb), &filesel ) );
  filesel.get_cancel_button()->clicked.connect( filesel.destroy.slot() );
  filesel.hide_fileop_buttons();
  if (icqclient.usingDefaultMap()) filesel.set_filename( TRANSLATIONS_DIR );
  else filesel.set_filename( icqclient.getTranslationMapFileName() );
  filesel.set_modal(true);
  filesel.show();
  Gtk::Main::run();
}

void SettingsDialog::trans_ok_cb(Gtk::FileSelection *filesel) {
  icqclient.setTranslationMap(filesel->get_filename());
  trans_l.set_text(icqclient.getTranslationMapName());
  filesel->destroy();
}

void SettingsDialog::fontsel_cb(int n) {
  Gtk::FontSelectionDialog fontsel( (n == 0 ? "Header Font Selection" : "Text Font Selection" ) );
  fontsel.destroy.connect( Gtk::Main::quit.slot() );
  fontsel.get_ok_button()->clicked.connect( bind( bind(slot( this, &SettingsDialog::fontsel_ok_cb ), n), &fontsel ) );
  fontsel.get_cancel_button()->clicked.connect( fontsel.destroy.slot() );
  if (n == 0) {
    fontsel.set_font_name( message_header_font );
  } else {
    fontsel.set_font_name( message_text_font );
  }

  fontsel.set_modal(true);
  fontsel.show();
  Gtk::Main::run();
}

void SettingsDialog::fontsel_ok_cb(Gtk::FontSelectionDialog *fontsel, int n) {
  GtkFontSelection* fs = const_cast<GtkFontSelection*>(fontsel->get_font_selection()->gtkobj());
  gchar *tmp = gtk_font_selection_get_font_name(fs);
  if(tmp != 0) {
    if (n == 0) {
      message_header_font = fontsel->get_font_name();
    } else {
      message_text_font = fontsel->get_font_name();
    }
  }
  else
    g_warning( "No font selected, neither font nor size was changed!" );
  fontsel->destroy();
}

void SettingsDialog::icons_cb() {
  string s = getIconsFilename();
  if( s.size() )
    g_icons.setIcons( s );
}
 
string SettingsDialog::getIconsFilename() {
  string filename = icons_combo.get_entry()->get_text();
  if (filename != "Default") filename = ICONS_DIR + filename + "/";
  return filename;
}

void SettingsDialog::away_up_button_cb()
{
  if (away_current_item_number > 0
      && away_current_item_number < away_response_msg_list.size() ) {
    // move selection
    away_response_list.row(away_current_item_number-1).select();

    // swap list items
    swap(away_response_msg_list[away_current_item_number], away_response_msg_list[away_current_item_number+1]);
    swap(away_response_label_list[away_current_item_number], away_response_label_list[away_current_item_number+1]);

    // update clist
    away_response_list.row( away_current_item_number+1 )[0].set_text( away_response_label_list[away_current_item_number+1] );
    away_response_list.row( away_current_item_number )[0].set_text( away_response_label_list[away_current_item_number] );

    // restore label, msg
    away_response_select_row(away_current_item_number);
  }
}

void SettingsDialog::away_down_button_cb()
{
  if (away_current_item_number < away_response_msg_list.size() - 1 ) {
    // move selection
    away_response_list.row(away_current_item_number+1).select();

    // swap list items
    swap(away_response_msg_list[away_current_item_number], away_response_msg_list[away_current_item_number-1]);
    swap(away_response_label_list[away_current_item_number], away_response_label_list[away_current_item_number-1]);

    // update clist
    away_response_list.row( away_current_item_number-1 )[0].set_text( away_response_label_list[away_current_item_number-1] );
    away_response_list.row( away_current_item_number )[0].set_text( away_response_label_list[away_current_item_number] );

    // restore label, msg
    away_response_select_row(away_current_item_number);
  }
}

void SettingsDialog::away_remove_button_cb()
{
  if (away_current_item_number < away_response_msg_list.size()) {
    away_response_msg_list.erase( away_response_msg_list.begin() + away_current_item_number );
    away_response_label_list.erase( away_response_label_list.begin() + away_current_item_number );

    unsigned int n = away_current_item_number;
    away_current_item_number = away_response_msg_list.size();
    away_response_list.remove_row(n);
    // remove_row fires off a select_cb which will screw stuff up unless
    // away_current_item_number is 'tweaked'
  }
}

void SettingsDialog::away_response_buttons_update()
{
  if (away_current_item_number >= away_response_msg_list.size()) {
    away_remove_button.set_sensitive(false);
    away_up_button.set_sensitive( false );
    away_down_button.set_sensitive(false);
  } else {
    away_up_button.set_sensitive( away_current_item_number != 0 );
    away_down_button.set_sensitive( away_current_item_number != away_response_msg_list.size() - 1 );
    away_remove_button.set_sensitive(true);
  }
}

void SettingsDialog::away_response_label_edit()
{
  if (away_response_label_edit_dead) return;
  
  if (away_current_item_number >= away_response_msg_list.size()) {
    // add as new
    away_response_label_list.push_back( away_response_label_entry.get_text() );
    away_response_msg_list.push_back( away_response_msg.get_chars(0, -1) );

    using namespace Gtk::CList_Helpers;

    RowList& il = away_response_list.rows();
    RowList::iterator ri = il.end();
    --ri;
    vector<string> a;
    a.push_back( away_response_label_entry.get_text() );
    ri = il.insert( ri, a );

    // select it
    (*ri).select();
  } else {

    // update label
    away_response_list.row( away_current_item_number )[0].set_text( away_response_label_entry.get_text() );

  }
}

void SettingsDialog::away_response_select_row(unsigned int row)
{
  if (row >= away_response_msg_list.size()) {
    // new selected
    away_response_msg.delete_text(0,-1);

    // connecting/disconnecting the callback doesn't seem to work correctly
    away_response_label_edit_dead = true;
    away_response_label_entry.delete_text(0,-1);
    away_response_label_edit_dead = false;

    away_response_label_entry.grab_focus();
    
  } else {
    away_response_msg.freeze();
    away_response_msg.delete_text(0,-1);
    away_response_msg.insert( away_response_msg_list[row] );
    away_response_msg.thaw();    

    away_response_label_edit_dead = true;
    away_response_label_entry.set_text( away_response_label_list[row] );
    away_response_label_edit_dead = false;
  }

  away_current_item_number = row;
  away_response_buttons_update();
}

void SettingsDialog::away_response_list_select_row_cb(gint p0, gint, GdkEvent *)
{
  if (away_current_item_number < away_response_msg_list.size()) {
    // save old
    away_response_msg_list[away_current_item_number] = away_response_msg.get_chars(0, -1);
    away_response_label_list[away_current_item_number] = away_response_label_entry.get_text();
  }

  away_response_select_row(p0);
}

gint SettingsDialog::delete_event_impl(GdkEventAny*)
{
  Gtk::Main::quit();
  return false;
}

void SettingsDialog::raise_away_status_tab() {
  notebook.set_page(3);
}

void SettingsDialog::spinner_changed(Gtk::SpinButton *sb)
{
  if (sb->get_text() == "0")
    sb->set_text("Disabled");
}
