/*
 * SettingsDialog
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
#include "Client.h"
#include "Dir.h"
#include "Icons.h"

#include <gtk--/box.h>
#include <gtk--/table.h>
#include <gtk--/frame.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/optionmenu.h>
#include <gtk--/menu.h>
#include <gtk--/menushell.h>
#include <gtk--/adjustment.h>

using SigC::slot;
using SigC::bind;

SettingsDialog::SettingsDialog()
  : Gtk::Dialog(),
    okay("OK"), cancel("Cancel"),
    away_autoposition("Autoposition away messages dialog", 0),
    reconnect_checkbox("Auto Reconnect", 0),
    log_info("Information", 0),
    log_warn("Warnings", 0),
    log_error("Errors", 0),
    log_packet("Server Packets", 0),
    log_directpacket("Direct Packets", 0),
    log_to_nowhere("The void", 0),
    log_to_console("Console", 0),
    log_to_file("File (~/.ickle/messages.log)", 0),
    log_to_consolefile("Selected to console, all to file", 0),
    network_override_port("Override server redirect port with login port", 0),
    message_autopopup("Autopopup on incoming message", 0)
{
  set_title("Settings Dialog");
  set_modal(true);

  okay.clicked.connect(slot(this,&SettingsDialog::okay_cb));
  cancel.clicked.connect(slot(this,&SettingsDialog::cancel_cb));

  notebook.set_tab_pos(GTK_POS_TOP);

  Gtk::HBox *hbox = get_action_area();
  hbox->pack_start(okay, true, true, 0);
  hbox->pack_start(cancel, true, true, 0);

  // ---------------- General tab -------------------------

  Gtk::Table *ftable = manage( new Gtk::Table( 2, 2, true ) );
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
    for (int n = STATUS_ONLINE; n <= STATUS_OFFLINE; n++)
      ml.push_back( MenuElem( Status_text[n], bind( slot(this, &SettingsDialog::setStatus), Status(n) ) ) );
  }
  m_status = Status(g_settings.getValueUnsignedInt("autoconnect"));
  m->set_active( m_status - STATUS_ONLINE );
  autoconnect_om->set_menu(*m);

  table->attach( *autoconnect_om, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  unsigned char n_retr = g_settings.getValueUnsignedChar("reconnect_retries");

  reconnect_label = manage( new Gtk::Label( "Retries", 0 ) );
  Gtk::Adjustment *adj = manage( new Gtk::Adjustment( (n_retr == 0 ? 1 : n_retr), 1.0, 10.0 ) );
  reconnect_spinner = manage( new Gtk::SpinButton( *adj, 1.0, 0 ) );

  reconnect_checkbox.toggled.connect( slot( this, &SettingsDialog::reconnect_toggle_cb ) );
  reconnect_checkbox.set_active( n_retr > 0 );
  table->attach( reconnect_checkbox, 0, 2, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  table->attach( *reconnect_label, 0, 1, 4, 5, GTK_FILL | GTK_EXPAND, 0);

  table->attach( *reconnect_spinner, 1, 2, 4, 5, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  if ( n_retr == 0 ) {
    reconnect_label->set_sensitive(false);
    reconnect_spinner->set_sensitive(false);
  }

  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);

  Gtk::Frame *frame = manage( new Gtk::Frame("Login Details") );
  frame->add(*table);

  ftable->attach( *frame, 0, 1, 0, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  frame = manage( new Gtk::Frame("Translation") );
  trans_l.set_text( icqclient.getTranslationMapName() );
  trans_b.set_border_width(10);
  trans_b.add(trans_l);
  trans_b.clicked.connect( slot( this, &SettingsDialog::trans_cb ) );
  frame->add(trans_b);

  ftable->attach( *frame, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  frame = manage( new Gtk::Frame("Icons") );
  icons_list.set_selection_mode(GTK_SELECTION_BROWSE);
  {
    string current = g_settings.getValueString("icons_dir");

    using namespace Gtk::List_Helpers;
    using Gtk::ListItem;
    ItemList& il = icons_list.items();
    il.push_back( * manage( new ListItem("Default") ) );
    Dir dir;
    dir.setDirectoriesOnly(true);
    dir.list( ICONS_DIR + "*" );
    Dir::iterator iter = dir.begin();
    int n = 1;
    while (iter != dir.end()) {
      string name, filename = *iter;
      string::size_type pos = filename.rfind('/');
      if ( pos == string::npos) name = filename;
      else name = string(filename,pos+1);

      il.push_back( * manage( new ListItem( name ) ) );
      if (current == filename+"/") icons_list.select_item(n);
      ++iter;
      ++n;
    }
    if (icons_list.selection().size() == 0) icons_list.select_item(0);
  }

  icons_list.selection_changed.connect( slot( this, &SettingsDialog::icons_cb ) );

  Gtk::ScrolledWindow *scrolled_window = manage(new Gtk::ScrolledWindow());
  scrolled_window->set_usize(300, 200);
  scrolled_window->set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  scrolled_window->add_with_viewport(icons_list);
  frame->add(*scrolled_window);

  ftable->attach( *frame, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  label = manage( new Gtk::Label( "General" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *ftable, *label )  );
  // ------------------------------------------------------------

  // ---------------- Events tab --------------------------

  table = manage( new Gtk::Table( 2, 4, false ) );
  
  label = manage( new Gtk::Label( "Below you can enter in commands to be executed when you receive an event. "
				  "Leave them blank if you don't want anything to happen. ", 0 ) );
  label->set_line_wrap(true);
  table->attach( *label, 0, 2, 0, 1, GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL | GTK_EXPAND | GTK_SHRINK);

  label = manage( new Gtk::Label( "Message Event", 0 ) );
  table->attach( *label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, 0);
  event_message_entry.set_text( g_settings.getValueString("event_message") );
  table->attach( event_message_entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "URL Event", 0 ) );
  table->attach( *label, 0, 1, 2, 3, GTK_FILL | GTK_EXPAND, 0);
  event_url_entry.set_text( g_settings.getValueString("event_url") );
  table->attach( event_url_entry, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "SMS Event", 0 ) );
  table->attach( *label, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND, 0);
  event_sms_entry.set_text( g_settings.getValueString("event_sms") );
  table->attach( event_sms_entry, 1, 2, 3, 4, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0);

  label = manage( new Gtk::Label( "Events" ) );
  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);

  frame = manage( new Gtk::Frame("Events") );
  frame->set_border_width(5);
  frame->add(*table);

  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *frame, *label )  );

  // ---------------------------------------------------------

  // ------------------ Message Box --------------------------

  table = manage( new Gtk::Table( 2, 1, false ) );
  
  message_autopopup.set_active( g_settings.getValueBool("message_autopopup") );
  table->attach( message_autopopup, 0, 2, 1, 2, GTK_FILL | GTK_EXPAND, 0);

  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);

  frame = manage( new Gtk::Frame("Message Box") );
  frame->set_border_width(5);
  frame->add(*table);

  label = manage( new Gtk::Label( "Message Box" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *frame, *label )  );

  // ---------------------------------------------------------

  // ------------------ Away Messages ------------------------

  table = manage( new Gtk::Table( 2, 1, false ) );
  
  away_autoposition.set_active( g_settings.getValueBool("away_autoposition") );
  table->attach( away_autoposition, 0, 2, 1, 2, GTK_FILL | GTK_EXPAND, 0);

  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);

  frame = manage( new Gtk::Frame("Away Messages") );
  frame->set_border_width(5);
  frame->add(*table);

  label = manage( new Gtk::Label( "Away Messages" ) );
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

  table = manage( new Gtk::Table( 2, 3, false ) );

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

  table->set_row_spacings(10);
  table->set_col_spacings(10);
  table->set_border_width(10);

  frame = manage( new Gtk::Frame("Network") );
  frame->set_border_width(5);
  frame->add(*table);

  label = manage( new Gtk::Label( "Network" ) );
  notebook.pages().push_back(  Gtk::Notebook_Helpers::TabElem( *frame, *label )  );

  // ---------------------------------------------------------

  Gtk::VBox *vbox = get_vbox();
  vbox->pack_start( notebook, true, true );


  set_border_width(10);
  set_usize(500, 300);
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

  // ------------ Events tab -----------------------
  g_settings.setValue("event_message", event_message_entry.get_text());
  g_settings.setValue("event_url", event_url_entry.get_text());
  g_settings.setValue("event_sms", event_sms_entry.get_text());

  // ------------ Message Box tab ------------------
  g_settings.setValue("message_autopopup", message_autopopup.get_active() );

  // ------------ Away Messages tab ----------------
  g_settings.setValue("away_autoposition", away_autoposition.get_active() );

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

  icqclient.setLoginServerHost( g_settings.getValueString("network_login_host") );
  icqclient.setLoginServerPort( g_settings.getValueUnsignedShort("network_login_port") );
  if (g_settings.getValueBool("network_override_port")) {
    icqclient.setBOSServerOverridePort(true);
    icqclient.setBOSServerPort( g_settings.getValueUnsignedShort("network_login_port") );
  }
}

unsigned int SettingsDialog::getUIN() const {
  return ICQ2000::Contact::StringtoUIN(uin_entry.get_text());
}

string SettingsDialog::getPassword() const {
  return password_entry.get_text();
}

void SettingsDialog::reconnect_toggle_cb() {
  if ( reconnect_checkbox.get_active() ) {
    reconnect_label->set_sensitive(true);
    reconnect_spinner->set_sensitive(true);
  } else {
    reconnect_label->set_sensitive(false);
    reconnect_spinner->set_sensitive(false);
  }
}

void SettingsDialog::okay_cb() {
  Gtk::Main::quit();
  finished_okay = true;
}

void SettingsDialog::cancel_cb() {
  Gtk::Main::quit();
  finished_okay = false;
  string m_old_icons_dir = g_settings.getValueString("icons_dir");
  if ( getIconsFilename() != m_old_icons_dir ) {
    // restore icons
    g_icons.setIcons( m_old_icons_dir );
  }
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

void SettingsDialog::icons_cb() {
  string s = getIconsFilename();
  if( s.size() )
    g_icons.setIcons( s );
}
 
string SettingsDialog::getIconsFilename() {
  Gtk::List::SelectionList &slist = icons_list.selection();
  if (slist.empty()) return "";

  Gtk::List::SelectionList::iterator iter = slist.begin();
  
  string filename = dynamic_cast<Gtk::Label*>((*iter)->get_child())->get();
  if (filename != "Default") filename = ICONS_DIR + filename + "/";
  return filename;
}


void SettingsDialog::trans_ok_cb(Gtk::FileSelection *filesel) {
  icqclient.setTranslationMap(filesel->get_filename());
  trans_l.set_text(icqclient.getTranslationMapName());
  filesel->destroy();
}
