/* $Id: IckleClient.cpp,v 1.100 2002-04-21 14:56:19 barnabygray Exp $
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

#include "IckleClient.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#include <stdexcept>
#include <fstream>
#include <signal.h>
#include "sstream_fix.h"

#include "main.h"
#include "Icons.h"
#include "Dir.h"
#include "EventSubstituter.h"
#include "WizardDialog.h"

#include <libicq2000/Client.h>

#include <time.h>

using std::string;
using std::ostringstream;
using std::istringstream;
using std::fstream;
using std::ofstream;
using std::cout;
using std::endl;
using std::map;
using std::runtime_error;

using ICQ2000::ContactRef;

IckleClient::IckleClient(int argc, char* argv[])
  : m_message_queue(),
    m_event_system(m_message_queue),
    gui( m_message_queue ),
    
#ifdef GNOME_ICKLE
    applet(m_message_queue),
#endif
#ifdef CONTROL_SOCKET
    ctrl(*this),
#endif
    status(ICQ2000::STATUS_OFFLINE)
{
  // process command line parameters
  processCommandLine(argc,argv);

  // make sure ickle isn't already running with the current configuration directory
  check_pid_file();

#ifdef GNOME_ICKLE
  // initialize GNOME applet
  applet.init(argc, argv, gui);
  applet.user_popup.connect( slot( this,&IckleClient::user_popup_cb ) );
  applet.exit.connect(slot(this,&IckleClient::exit_cb));
#endif

#ifdef CONTROL_SOCKET
  // initialize control socket
  ctrl.init();
#endif

  // let us know when the gui is destroyed
  gui.destroy.connect(slot(this,&IckleClient::quit));
  gui.delete_event.connect(slot(this,&IckleClient::close_cb));
  gui.exit.connect(slot(this,&IckleClient::exit_cb));
  
  // set up libICQ2000 Callbacks
  // -- callbacks into IckleClient

  icqclient.connected.connect(slot(this,&IckleClient::connected_cb));
  icqclient.disconnected.connect(slot(this,&IckleClient::disconnected_cb));
  icqclient.logger.connect(slot(this,&IckleClient::logger_cb));
  icqclient.contactlist.connect(slot(this,&IckleClient::contactlist_cb));
  icqclient.messaged.connect(slot(this,&IckleClient::message_cb));
  icqclient.messageack.connect(slot(this,&IckleClient::messageack_cb));
  icqclient.contact_status_change_signal.connect(slot(m_event_system,&EventSystem::status_change_cb));
  icqclient.socket.connect(slot(this,&IckleClient::socket_cb));
  icqclient.want_auto_resp.connect(slot(this,&IckleClient::want_auto_resp_cb));

  // message queue callbacks
  m_message_queue.added.connect(slot(this,&IckleClient::queue_added_cb));
  m_message_queue.added.connect(slot(&m_event_system, &EventSystem::queue_added_cb));

  // set up GUI callbacks
  gui.settings_changed.connect(slot(this,&IckleClient::settings_changed_cb));
  gui.user_popup.connect( slot( this,&IckleClient::user_popup_cb ) );
  gui.getContactListView()->user_popup.connect( slot( this,&IckleClient::user_popup_cb ) );
  gui.getContactListView()->userinfo.connect( slot( this, &IckleClient::userinfo_cb ) );

  gui.send_event.connect(slot(this,&IckleClient::send_event_cb));

  gui.setDisplayTimes(true);

  loadSettings();
  loadSelfContact();

  gui.getContactListView()->load_sort_column ();

  // setup contact list
  loadContactList();

#ifdef GNOME_ICKLE
  if( !g_settings.getValueBool("hidegui_onstart") )
#endif
  gui.show_all();

  ICQ2000::Status st = ICQ2000::Status(g_settings.getValueUnsignedInt("autoconnect"));
  // give gtk some time to breathe before we do an auto connect - dns
  // lookup will block which would prevent them seeing anything for as
  // long as that takes
  if (st != ICQ2000::STATUS_OFFLINE) Gtk::Main::idle.connect( bind( slot( this, &IckleClient::idle_connect_cb ), st ) );

  if( g_settings.getValueUnsignedInt("uin") == 0 ) {
    WizardDialog wiz;
    if( wiz.run() ) // if we got an uin, automatically go online to allow the user to set his info
      Gtk::Main::idle.connect( bind( slot( this, &IckleClient::idle_connect_cb ), ICQ2000::STATUS_ONLINE ) );
  }
}

IckleClient::~IckleClient() {
  // free History objects
  for( map<unsigned int, History *>::iterator itr = m_histmap.begin(); itr != m_histmap.end(); ++itr )
    delete itr->second;

  // remove the ickle.pid file
  if (!PID_FILENAME.empty()) unlink(PID_FILENAME.c_str());

}

void IckleClient::loadContactList() {
  Dir dir;
  dir.list( CONTACT_DIR + "*.user" );

  Dir::iterator dirit = dir.begin();
  while( dirit != dir.end() ) {
    try {
      loadContact( *dirit, false );
    } catch(runtime_error& e) {
      SignalLog(ICQ2000::LogEvent::WARN, e.what());
    }
    ++dirit;
  }

}

void IckleClient::processCommandLine(int argc, char* argv[]) {
  int i = 0;
  while ( ( i = getopt( argc, argv, "hb:" ) ) > 0) {

    switch(i) {
    case '?':
#ifdef GNOME_ICKLE
      break;  // We'll ignore any unknown options when run as an applet
#endif      
    case 'h': // help
      usageInstructions(argv[0]);
      exit(0);
      break;
    case 'b': // base dir
      BASE_DIR = string(optarg) + "/";
      break;
    }
  }
	 
  if (BASE_DIR.empty()) {
    // default to ~/.ickle/ if home is defined, otherwise just .ickle/ in the current directory
    char *dir = getenv("HOME");
    if (dir == NULL) dir = getenv("PWD");

    if (dir != NULL) {
      BASE_DIR = string(dir) + "/.ickle/";
    } else {
      BASE_DIR = ".ickle/";
    }
    
  }

  CONTACT_DIR = BASE_DIR + "contacts/";
  DATA_DIR = string(PKGDATADIR) + "/";
  TRANSLATIONS_DIR = DATA_DIR + "translations/";
  ICONS_DIR = DATA_DIR + "icons/";
  PID_FILENAME = BASE_DIR + "ickle.pid";
}

void IckleClient::usageInstructions(const char* progname) {
  cout << "ickle version " << ICKLE_VERSION << endl
       << "Usage: " << progname << " [-h] [-b dir]" << endl << endl
       << " -h : the help screen you are seeing" << endl
       << " -b : use a different configuration directory (~/.ickle/ is the default)" << endl
       << endl;
}


void IckleClient::SignalLog(ICQ2000::LogEvent::LogType type, const string& msg) {
  ICQ2000::LogEvent ev(type,msg);
  logger_cb(&ev);
}
  
void IckleClient::loadSettings() {
  // load in settings
  try {
    g_settings.load(BASE_DIR + "ickle.conf");
  } catch (runtime_error& e) {
    ostringstream ostr;
    ostr << "Couldn't open " << BASE_DIR << "ickle.conf, using default settings" << endl
	 << "This is probably the first time you've run ickle.";
    SignalLog(ICQ2000::LogEvent::WARN, ostr.str());
  }

  /*
   * Default settings & limits to numerical values
   */
  g_settings.defaultValueBool("away_autoposition", true);
  g_settings.defaultValueUnsignedShort("auto_away", 0, 0, 65535);
  g_settings.defaultValueUnsignedShort("auto_na", 0, 0, 65535);
  g_settings.defaultValueUnsignedInt("reconnect_retries", 2, 0, 10);

  g_settings.defaultValueBool("window_status_icons", true);

  /* Event settings */
  g_settings.defaultValueUnsignedInt("event_repetition_threshold", 100);
  g_settings.defaultValueBool("event_execute_repeated", false);

  /* Default log settings */
  g_settings.defaultValueBool("log_to_console", true);
  g_settings.defaultValueBool("log_to_file", false);
  g_settings.defaultValueBool("log_info", false);
  g_settings.defaultValueBool("log_error", true);
  g_settings.defaultValueBool("log_warn", true);
  g_settings.defaultValueBool("log_packet", false);
  g_settings.defaultValueBool("log_directpacket", false);

  /* Default geometry for main window */
  g_settings.defaultValueUnsignedInt("geometry_width", 130, 30, 1000);
  g_settings.defaultValueUnsignedInt("geometry_height", 300, 30, 2000);
  g_settings.defaultValueUnsignedInt("geometry_x", 50, 0);
  g_settings.defaultValueUnsignedInt("geometry_y", 50, 0);

  /* Default connection/network settings */
  g_settings.defaultValueUnsignedInt("autoconnect", ICQ2000::STATUS_OFFLINE, ICQ2000::STATUS_ONLINE, ICQ2000::STATUS_OFFLINE);
  g_settings.defaultValueString("network_login_host", "login.icq.com");
  g_settings.defaultValueUnsignedShort("network_login_port", 5190, 1, 65535);
  g_settings.defaultValueBool("network_override_port", false);
  g_settings.defaultValueBool("network_in_dc", true);
  g_settings.defaultValueBool("network_out_dc", true);
  g_settings.defaultValueBool("network_use_portrange", false);
  g_settings.defaultValueUnsignedShort("network_lower_bind_port", 9000, 1, 65535);
  g_settings.defaultValueUnsignedShort("network_upper_bind_port", 9010, 1, 65535);

  /* Default SMTP server settings */
  g_settings.defaultValueBool("network_smtp", true);
  g_settings.defaultValueString("network_smtp_host", "localhost");
  g_settings.defaultValueUnsignedShort("network_smtp_port", 25, 1, 65535);

  /* Default message box settings */
  g_settings.defaultValueBool("message_autopopup", false);
  g_settings.defaultValueBool("message_autoraise", true);
  g_settings.defaultValueBool("message_autoclose", false);
  g_settings.defaultValueUnsignedInt("history_shownr", 10, 1, 255);
  g_settings.defaultValueString("message_header_font", "-*-helvetica-bold-r-*-*-*-*-*-*-*-*-*-*");
  g_settings.defaultValueString("message_text_font", "-*-helvetica-medium-r-*-*-*-*-*-*-*-*-*-*");

  /* Default size 0 means that we leave it up to the packed widgets to decide size */
  g_settings.defaultValueUnsignedInt("message_box_width", 0, 0, 2000);
  g_settings.defaultValueUnsignedInt("message_box_height", 0, 0, 2000);
  g_settings.defaultValueUnsignedInt("message_box_pane_position", 0, 0, 2000);

  g_settings.defaultValueBool("status_classic_invisibility", false);

  g_settings.defaultValueString("last_away_response", "User is currently not available\nYou can leave him/her a message");
  g_settings.defaultValueBool("set_away_response_dialog", true);
  g_settings.defaultValueBool("set_away_response_timeout", true);
  g_settings.defaultValueBool("mouse_single_click", false);
  g_settings.defaultValueBool("mouse_check_away_click", true);
  g_settings.defaultValueUnsignedInt("sort_contact_list_column", 0, 0, 1000);
  g_settings.defaultValueBool("spell_check", true);
  g_settings.defaultValueBool("spell_check_aspell", false);
  g_settings.defaultValueString("spell_check_lang","");
  g_settings.defaultValueBool("initial_userinfo_done", false );

  g_settings.defaultValueUnsignedInt("no_autoresponses", 1, 1, 100);
  g_settings.defaultValueString("autoresponse_1_label", "Away");
  g_settings.defaultValueString("autoresponse_1_text", "Hello %a, I'm away at the moment.");

#ifdef GNOME_ICKLE
  g_settings.defaultValueBool("hidegui_onstart", false);
#endif
  
  // Set settings in library
  icqclient.setUIN(g_settings.getValueUnsignedInt("uin"));
  icqclient.setPassword(g_settings.getValueString("password"));
  icqclient.setTranslationMap( g_settings.getValueString("translation_map") );
  icqclient.setLoginServerHost( g_settings.getValueString("network_login_host") );
  icqclient.setLoginServerPort( g_settings.getValueUnsignedShort("network_login_port") );
  if (g_settings.getValueBool("network_override_port")) {
    icqclient.setBOSServerOverridePort(true);
    icqclient.setBOSServerPort( g_settings.getValueUnsignedShort("network_login_port") );
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
    
  icqclient.setAcceptInDC( g_settings.getValueBool("network_in_dc") );
  icqclient.setUseOutDC( g_settings.getValueBool("network_out_dc") );

  // --

  // Set contact list stuff
  int width, height, x, y;
  width = g_settings.getValueUnsignedInt("geometry_width");
  height = g_settings.getValueUnsignedInt("geometry_height");
  x = g_settings.getValueUnsignedInt("geometry_x");
  y = g_settings.getValueUnsignedInt("geometry_y");

  int swidth, sheight;
  swidth = gdk_screen_width();
  sheight = gdk_screen_height();

  if (width > swidth) width = swidth;
  if (height > sheight) height = sheight;

  if (x + width > swidth) x = swidth-width;
  if (y + height > sheight) y = sheight-height;

  gui.setGeometry(x, y, width, height);
  
  ContactListView* clist = gui.getContactListView();
  if (clist) {
    clist->setSingleClick(g_settings.getValueBool("mouse_single_click"));
    clist->setCheckAwayClick(g_settings.getValueBool("mouse_check_away_click"));
  }

  g_icons.setIcons( g_settings.getValueString("icons_dir") );
  
  m_retries = g_settings.getValueUnsignedChar("reconnect_retries");

  // Load up auto response in case autoconnect = away/na/etc.
  string auto_response = g_settings.getValueString("last_auto_response");
  gui.setAutoResponse(auto_response);

  if (g_settings.getValueBool("spell_check"))
    gui.spell_check_setup();
    
}

void IckleClient::saveSettings() {
  int width, height, x, y;
  gui.get_window().get_root_origin(x, y);
  gui.get_window().get_size(width, height);
  g_settings.setValue( "geometry_x", x );
  g_settings.setValue( "geometry_y", y );
  g_settings.setValue( "geometry_width", width );
  g_settings.setValue( "geometry_height", height );

  if (!mkdir_BASE_DIR()) return;
  
  string ickle_conf = BASE_DIR + "ickle.conf";

  // set umask to secure value, so that if ickle.conf doesn't exist, and is created it will be safe.
  mode_t old_umask = umask(0077);

  try {
    g_settings.save(ickle_conf);
  } catch(runtime_error& e) {
    ostringstream ostr;
    ostr << "Couldn't save " << BASE_DIR << "ickle.conf";
    SignalLog(ICQ2000::LogEvent::ERROR, ostr.str());
  }

  umask(old_umask);

  // ensure permissions on ickle.conf are secure
  if ( chmod( ickle_conf.c_str(), S_IRUSR | S_IWUSR ) == -1 ) {
    ostringstream ostr;
    ostr << "The permissions on " << ickle_conf << " couldn't be set to 0600. Your ICQ password is vulnerable!";
    SignalLog(ICQ2000::LogEvent::ERROR, ostr.str());
  }

  // save self-contact
  saveSelfContact();

  // save contact-specific settings
  for( map<unsigned int, string>::iterator itr = m_settingsmap.begin(); itr != m_settingsmap.end(); ++itr ) {
    ContactRef c = icqclient.getContact( itr->first );
    if (c.get() != NULL) saveContact( c, itr->second );
  }
}

void IckleClient::exit_cb() {
  /*
   * These both need to be done while the widget
   * still exists, in IckleClient::quit is too late
   */
  saveSettings();
  icqclient.setStatus(ICQ2000::STATUS_OFFLINE);
}

gint IckleClient::close_cb(GdkEventAny*) {
#ifndef GNOME_ICKLE
  exit_cb();
#endif
  return false;
}

void IckleClient::quit() {
#ifdef CONTROL_SOCKET
  ctrl.quit();
#endif
#ifdef GNOME_ICKLE
  applet.quit();
#else
  Gtk::Main::quit();
#endif
}

void IckleClient::connected_cb(ICQ2000::ConnectedEvent *c) {
  /* the library needs to be polled regularly
   * to ensure timeouts are respected and the server is pinged every minute
   * A 5 second interval is sensible for this
   */
  poll_server_cnt = Gtk::Main::timeout.connect( slot( this, &IckleClient::poll_server_cb ), 5000 );
  m_retries = g_settings.getValueUnsignedChar("reconnect_retries");

  if( !g_settings.getValueBool("initial_userinfo_done") ) {
    gui.my_user_info_cb();
    g_settings.setValue("initial_userinfo_done", true );
  }
}

void IckleClient::disconnected_cb(ICQ2000::DisconnectedEvent *c) {
  if (c->getReason() == ICQ2000::DisconnectedEvent::REQUESTED) {
    SignalLog(ICQ2000::LogEvent::INFO, "Disconnected as requested");
  } else {
    ostringstream ostr;
    ostr << "Problem connecting: ";
    switch(c->getReason()) {
    case ICQ2000::DisconnectedEvent::FAILED_LOWLEVEL:
      ostr << "Socket problems";
      break;
    case ICQ2000::DisconnectedEvent::FAILED_BADUSERNAME:
      ostr << "Bad Username";
      break;
    case ICQ2000::DisconnectedEvent::FAILED_TURBOING:
      ostr << "Turboing";
      break;
    case ICQ2000::DisconnectedEvent::FAILED_BADPASSWORD:
      ostr << "Bad Password";
      break;
    case ICQ2000::DisconnectedEvent::FAILED_MISMATCH_PASSWD:
      ostr << "Username and Password did not match";
      break;
    case ICQ2000::DisconnectedEvent::FAILED_UNKNOWN:
      ostr << "Unknown";
      break;
    case ICQ2000::DisconnectedEvent::FAILED_DUALLOGIN:
      ostr << "Dual login, disconnected";
      break;
    }
    SignalLog(ICQ2000::LogEvent::ERROR, ostr.str() );
  }

  // disconnect PingServer callback
  poll_server_cnt.disconnect();
  
  // do another switch here since we want to return early for some of these reasons
    switch(c->getReason()) {
    case ICQ2000::DisconnectedEvent::REQUESTED:
      return;
    case ICQ2000::DisconnectedEvent::FAILED_BADUSERNAME:
    case ICQ2000::DisconnectedEvent::FAILED_BADPASSWORD:
    case ICQ2000::DisconnectedEvent::FAILED_MISMATCH_PASSWD:
      gui.invalid_login_prompt();
      return;
    case ICQ2000::DisconnectedEvent::FAILED_TURBOING:
      gui.turboing_prompt();
      return;
    case ICQ2000::DisconnectedEvent::FAILED_DUALLOGIN:
      gui.duallogin_prompt();
      return;
    }

  if (m_retries > 0) {
    --m_retries;
    Gtk::Main::idle.connect( bind( slot( this, &IckleClient::idle_connect_cb ), ICQ2000::STATUS_ONLINE ) );
  }
  else {
    m_retries = g_settings.getValueUnsignedChar("reconnect_retries"); // reset m_retries

    if( c->getReason() == ICQ2000::DisconnectedEvent::FAILED_LOWLEVEL )
      gui.disconnect_lowlevel_prompt(m_retries);
    else if( c->getReason() == ICQ2000::DisconnectedEvent::FAILED_UNKNOWN )
      gui.disconnect_unknown_prompt(m_retries);
  }
}

gint IckleClient::idle_connect_cb(ICQ2000::Status s) {
  icqclient.setStatus( s );
  return 0;
}

void IckleClient::logger_file_cb(const string& msg) {
  string log_file = BASE_DIR + "messages.log";
  
  // set umask to secure value, so that if ickle.conf doesn't exist, and is created it will be safe.
  mode_t old_umask = umask(0077);

  ofstream of(log_file.c_str(), std::ios::out | std::ios::app);
  if (of) of << msg;
  of.close();

  umask(old_umask);

  // ensure permissions on log file are secure
  if ( chmod( log_file.c_str(), S_IRUSR | S_IWUSR ) == -1 ) {
    cout << "The permissions on " << log_file << " couldn't be set to 0600. Your ICQ password may be logged in this file, so it could be vulnerable!" << endl;
  }
}

void IckleClient::logger_cb(ICQ2000::LogEvent *c) {

  bool log_to_console = g_settings.getValueBool("log_to_console");
  bool log_to_file = g_settings.getValueBool("log_to_file");
  if (!log_to_console && !log_to_file) return;

  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char time_str[256];
  strftime(time_str, 255, "[%H:%M:%S] ", tm);

  if ( log_to_console && log_to_file ) {
    ostringstream ostr;
    ostr << time_str << c->getMessage() << endl;
    logger_file_cb( ostr.str() );
  }

  switch(c->getType()) {
  case ICQ2000::LogEvent::INFO:
    if (!g_settings.getValueBool("log_info")) return;
    break;
  case ICQ2000::LogEvent::ERROR:
    if (!g_settings.getValueBool("log_error")) return;
    break;
  case ICQ2000::LogEvent::WARN:
    if (!g_settings.getValueBool("log_warn")) return;
    break;
  case ICQ2000::LogEvent::PACKET:
    if (!g_settings.getValueBool("log_packet")) return;
    break;
  case ICQ2000::LogEvent::DIRECTPACKET:
    if (!g_settings.getValueBool("log_directpacket")) return;
    break;
  }

  if ( !log_to_console && log_to_file ) {
    ostringstream ostr;
    ostr << time_str << c->getMessage() << endl;
    logger_file_cb(ostr.str());
  }

  if (log_to_console) {
    cout << time_str;
    switch(c->getType()) {
    case ICQ2000::LogEvent::INFO:
      cout << "[34m (i)  ";
      break;
    case ICQ2000::LogEvent::ERROR:
      cout << "[31m :-<  ";
      break;
    case ICQ2000::LogEvent::WARN:
      cout << "[36m :-%  ";
      break;
    case ICQ2000::LogEvent::PACKET:
      cout << "[32m [+]  ";
      break;
    case ICQ2000::LogEvent::DIRECTPACKET:
      cout << "[32m {~}  ";
      break;
    }
    cout << c->getMessage() << endl;
    cout << "[39m";
  }

}

string IckleClient::get_unique_historyname() throw (runtime_error)
{
  char tmpl[255];
  int fd;
  
  snprintf( tmpl, sizeof(tmpl), "%smobilehistory.XXXXXX", CONTACT_DIR.c_str() );
  if( (fd = mkstemp( tmpl ) ) == -1 )
    throw( runtime_error("Could not generate temporary filename for history") );
  close(fd);
  return string ( tmpl + CONTACT_DIR.size() );
}

void IckleClient::socket_select_cb(int source, GdkInputCondition cond) {
  ostringstream ostr;
  /*
    ostr << "IckleClient::socket_cb " << source << " cond: " << cond;
    SignalLog(ICQ2000::LogEvent::INFO, ostr.str());
  */

  icqclient.socket_cb(source, (ICQ2000::SocketEvent::Mode)cond);
}

int IckleClient::poll_server_cb() {
  icqclient.Poll();
  return true;
}

void IckleClient::user_popup_cb(unsigned int uin) {
  ContactRef c = icqclient.getContact(uin);
  if (c.get() != NULL) {
    if (m_message_queue.get_contact_size(c) == 0)
      gui.popup_messagebox(c, m_histmap[c->getUIN()]);
    else
      gui.popup_next_event(c, m_histmap[c->getUIN()]);
  }
}

void IckleClient::userinfo_cb(unsigned int uin) {
  ContactRef c = icqclient.getContact(uin);
  if (c.get() != NULL) {
    gui.popup_userinfo(c);
  }
}

void IckleClient::send_event_cb(ICQ2000::MessageEvent *ev) {
  icqclient.SendEvent(ev);
}

void IckleClient::messageack_cb(ICQ2000::MessageEvent *ev) {
  if (ev->isFinished() && ev->isDelivered()
      && ev->getType() != ICQ2000::MessageEvent::AwayMessage) {

    unsigned int uin = ev->getContact()->getUIN();
    if (m_histmap.count(uin) == 0) {
      ostringstream ostr;
      ostr << "No history object for contact: " << uin;
      SignalLog(ICQ2000::LogEvent::ERROR, ostr.str());
    } else {
      m_histmap[ev->getContact()->getUIN()]->log(ev, false);
    }

  }

}

void IckleClient::socket_cb(ICQ2000::SocketEvent *ev) {
  
  if (dynamic_cast<ICQ2000::AddSocketHandleEvent*>(ev) != NULL) {
    ICQ2000::AddSocketHandleEvent *cev = dynamic_cast<ICQ2000::AddSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    /*
    ostringstream ostr;
    ostr << "Connecting socket " << fd;
    if (cev->isRead()) ostr << " for read";
    if (cev->isWrite()) ostr << " for write";
    if (cev->isException()) ostr << " for exception";
    SignalLog(ICQ2000::LogEvent::INFO, ostr.str());
    */

    if (m_sockets.count(fd) > 0) {
      // uh oh..
      SignalLog(ICQ2000::LogEvent::ERROR, "Problem: file descriptor already connected");
      m_sockets[fd].disconnect();
      m_sockets.erase(fd);
    }

    m_sockets[fd] = Gtk::Main::input.connect(slot(this, &IckleClient::socket_select_cb), fd,
					     (GdkInputCondition)
					     ((cev->isRead() ? GDK_INPUT_READ : 0) |
					      (cev->isWrite() ? GDK_INPUT_WRITE : 0) |
					      (cev->isException() ? GDK_INPUT_EXCEPTION : 0)));

  } else if (dynamic_cast<ICQ2000::RemoveSocketHandleEvent*>(ev) != NULL) {
    ICQ2000::RemoveSocketHandleEvent *cev = dynamic_cast<ICQ2000::RemoveSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    /*
    ostringstream ostr;
    ostr << "Disconnecting socket " << fd;
    SignalLog(ICQ2000::LogEvent::INFO, ostr.str());
    */

    if (m_sockets.count(fd) == 0) {
      SignalLog(ICQ2000::LogEvent::ERROR, "Problem: file descriptor not connected");
    } else {
      m_sockets[fd].disconnect();
      m_sockets.erase(fd);
    }
  }
}

void IckleClient::message_cb(ICQ2000::MessageEvent *ev) {
  ContactRef c = ev->getContact();

  /* this behaviour is a client choice - might be nice to make it
     customisable eventually, for the moment we don't accept in
     occupied or denied unless it is sent as to contact list or
     urgent.  ignore lists will be implemented here eventually */

  ICQ2000::Status s = icqclient.getStatus();
  ICQ2000::ICQMessageEvent *icq = dynamic_cast<ICQ2000::ICQMessageEvent*>(ev);
  if (icq != NULL) {
    if ((s == ICQ2000::STATUS_OCCUPIED || s == ICQ2000::STATUS_DND)
	&& !(icq->isUrgent() || icq->isToContactList())) {
      ev->setDelivered(false);
      
      if (s == ICQ2000::STATUS_OCCUPIED)
	ev->setDeliveryFailureReason( ICQ2000::MessageEvent::Failed_Occupied );
      else
	ev->setDeliveryFailureReason( ICQ2000::MessageEvent::Failed_DND );
      
      // not logged, not signalled, just ignored :-)
      return;
    }
  }

  ev->setDelivered(true);

  // add to contact list if not on list already
  if (!icqclient.getContactList().exists(ev->getContact()->getUIN())) {
    icqclient.addContact(ev->getContact());
    icqclient.fetchDetailContactInfo(ev->getContact());
  }

  // convert to a client event type and add to queue
  MessageEvent *ickle_ev = convert_libicq2000_event(ev);
  if (ickle_ev == NULL) {
    SignalLog(ICQ2000::LogEvent::INFO, "Unhandled libicq2000 event");
    return;
  }
  
  m_message_queue.add_to_queue(ickle_ev);

  unsigned int uin = ev->getContact()->getUIN();
  if (m_histmap.count(uin) == 0) {
    ostringstream ostr;
    ostr << "No history object for contact: " << uin;
    SignalLog(ICQ2000::LogEvent::ERROR, ostr.str());
  } else {
    m_histmap[ev->getContact()->getUIN()]->log(ev, true);
  }
  
}

void IckleClient::queue_added_cb(MessageEvent *ev)
{
  // anything ?
}

MessageEvent* IckleClient::convert_libicq2000_event(ICQ2000::MessageEvent *ev)
{
  ICQMessageEvent *ret = NULL;
  ContactRef c = ev->getContact();
  
  switch(ev->getType()) {
  case ICQ2000::MessageEvent::Normal:
  {
    ICQ2000::NormalMessageEvent *cev = static_cast<ICQ2000::NormalMessageEvent*>(ev);
    NormalICQMessageEvent *ret2 = new NormalICQMessageEvent( ev->getTime(), c, cev->getMessage(),
							     cev->getForeground(), cev->getBackground() );
    ret2->setMultiParty( cev->isMultiParty() );
    ret = ret2;
    break;
  }
  case ICQ2000::MessageEvent::URL:
  {
    ICQ2000::URLMessageEvent *cev = static_cast<ICQ2000::URLMessageEvent*>(ev);
    ret = new URLICQMessageEvent( ev->getTime(), c, cev->getMessage(),
				  cev->getURL() );
    break;
  }
  case ICQ2000::MessageEvent::SMS:
  {
    ICQ2000::SMSMessageEvent *cev = static_cast<ICQ2000::SMSMessageEvent*>(ev);
    ret = new SMSICQMessageEvent( ev->getTime(), c, cev->getMessage() );
    break;
  }
  case ICQ2000::MessageEvent::SMS_Receipt:
  {
    ICQ2000::SMSReceiptEvent *cev = static_cast<ICQ2000::SMSReceiptEvent*>(ev);
    ret = new SMSReceiptICQMessageEvent( ev->getTime(), c, cev->getMessage(),
					 cev->getMessageId(), cev->getSubmissionTime(),
					 cev->getDeliveryTime(), cev->delivered());
    break;
  }
  case ICQ2000::MessageEvent::AuthReq:
  {
    ICQ2000::AuthReqEvent *cev = static_cast<ICQ2000::AuthReqEvent*>(ev);
    ret = new AuthReqICQMessageEvent( ev->getTime(), c, cev->getMessage() );
    break;
  }
  case ICQ2000::MessageEvent::AuthAck:
  {
    ICQ2000::AuthAckEvent *cev = static_cast<ICQ2000::AuthAckEvent*>(ev);
    ret = new AuthAckICQMessageEvent( ev->getTime(), c, cev->getMessage(), cev->isGranted() );
    break;
  }
  case ICQ2000::MessageEvent::EmailEx:
  {
    ICQ2000::EmailExEvent *cev = static_cast<ICQ2000::EmailExEvent*>(ev);
    ret = new EmailExICQMessageEvent( ev->getTime(), c, cev->getMessage() );
    break;
  }
  case ICQ2000::MessageEvent::UserAdd:
  {
    ICQ2000::UserAddEvent *cev = static_cast<ICQ2000::UserAddEvent*>(ev);
    ret = new UserAddICQMessageEvent( ev->getTime(), c );
    break;
  }
  default:
    ret = NULL;
  }

  ICQ2000::ICQMessageEvent *icq = dynamic_cast<ICQ2000::ICQMessageEvent*>(ev);
  if (ret != NULL && icq != NULL) {
    ret->setUrgent( icq->isUrgent() );
    ret->setToContactList( icq->isToContactList() );
    ret->setDirect( icq->isDirect() );
    ret->setOffline( icq->isOfflineMessage() );
  }

  return ret;
}

void IckleClient::want_auto_resp_cb(ICQ2000::ICQMessageEvent *ev) {
  ContactRef c = ev->getContact();
  EventSubstituter evs(m_message_queue, c);
  evs << gui.getAutoResponse();
  ev->setAwayMessage(evs.str());
}

bool IckleClient::mkdir_BASE_DIR()
{
  if ( mkdir( BASE_DIR.c_str(), 0700 ) == -1 && errno != EEXIST ) {
    ostringstream ostr;
    ostr << "mkdir " << BASE_DIR << " failed: " << strerror(errno);
    SignalLog(ICQ2000::LogEvent::ERROR, ostr.str());
    return false;
  }
  return true;
}

void IckleClient::contactlist_cb(ICQ2000::ContactListEvent *ev) {
  ContactRef c = ev->getContact();

  if (ev->getType() == ICQ2000::ContactListEvent::UserAdded) {
    if (m_settingsmap.count(c->getUIN()) > 0) return;
    ostringstream ostr;

    ostr << CONTACT_DIR << c->getUIN() << ".user";

    int n = 0;
    struct stat fs;
    string filename;
    filename = ostr.str();

    // ensure uniqueness
    while ( stat( filename.c_str(), &fs ) == 0 ) {
      ostringstream ostr;
      n++;
      ostr << CONTACT_DIR << c->getUIN() << "-" << n << ".user";
      filename = ostr.str();
    }
    m_settingsmap[c->getUIN()] = filename;
    if( c->isICQContact() ) {
      ostringstream os;
      os << c->getUIN() << ".history";
      m_histmap[c->getUIN()] = new History( os.str() );
    }
    else
      m_histmap[c->getUIN()] = new History( get_unique_historyname() );

    if ( !mkdir_BASE_DIR() ) return;

    if ( mkdir( CONTACT_DIR.c_str(), 0700 ) == -1 && errno != EEXIST ) {
      ostringstream ostr;
      ostr << "mkdir " << CONTACT_DIR << " failed: " << strerror(errno);
      SignalLog(ICQ2000::LogEvent::ERROR, ostr.str());
      return;
    }
    
    saveContact( c, m_settingsmap[c->getUIN()] );
    
  }
  else if (ev->getType() == ICQ2000::ContactListEvent::UserRemoved) {
    // delete .user file for this contact
    unlink( m_settingsmap[c->getUIN()].c_str() );

    // remove history file for mobile users as well, we will not be able to correctly
    // reuse this history file if the same user is added anyway
    if( !c->isICQContact() )
      unlink( string(CONTACT_DIR + m_histmap[c->getUIN()]->getFilename()).c_str() );

    m_histmap.erase(c->getUIN());
    m_settingsmap.erase(c->getUIN());

  }
}

void IckleClient::settings_changed_cb() {
  saveSettings();
}

void IckleClient::saveContact(ContactRef c, const string& s)
{
  Settings user;
  user.setValue( "alias", c->getAlias() );
  if (c->isICQContact())
    user.setValue( "uin", c->getUIN() );
  user.setValue( "mobile_no", c->getMobileNo() );
  user.setValue( "firstname", c->getFirstName() );
  user.setValue( "lastname", c->getLastName() );
  user.setValue( "email", c->getEmail() );

  // Main Home Info
  ICQ2000::Contact::MainHomeInfo& mhi = c->getMainHomeInfo();
  user.setValue( "city", mhi.city );
  user.setValue( "state", mhi.state );
  user.setValue( "phone", mhi.phone );
  user.setValue( "fax", mhi.fax );
  user.setValue( "street", mhi.street );
  user.setValue( "zip", mhi.zip );
  user.setValue( "country", mhi.country );
  user.setValue( "gmt", mhi.timezone );
    
  // Homepage Info
  ICQ2000::Contact::HomepageInfo& hpi = c->getHomepageInfo();
  user.setValue( "age", hpi.age );
  user.setValue( "sex", hpi.sex );
  user.setValue( "homepage", hpi.homepage );
  user.setValue( "birth_year", hpi.birth_year );
  user.setValue( "birth_month", hpi.birth_month );
  user.setValue( "birth_day", hpi.birth_day );
  user.setValue( "lang1", hpi.lang1 );
  user.setValue( "lang2", hpi.lang2 );
  user.setValue( "lang3", hpi.lang3 );
    
  // About Info
  user.setValue( "about", c->getAboutInfo() );

  // Stats
  user.setValue( "last_signon_time", c->get_signon_time() );
  user.setValue( "last_seen_online_time", c->get_last_online_time() );
  user.setValue( "last_status_change_time", c->get_last_status_change_time() );
  user.setValue( "last_message_time", c->get_last_message_time() );
  user.setValue( "last_away_msg_check_time", c->get_last_away_msg_check_time() );

  try {
    user.save(s);
  } catch(runtime_error& e) {
    SignalLog(ICQ2000::LogEvent::ERROR, e.what());
  }
}

void IckleClient::saveSelfContact()
{
  saveContact( icqclient.getSelfContact(), BASE_DIR + "self.user" );
}

void IckleClient::loadContact(const string& s, bool self)
{
  Settings cs;
  cs.load(s);
  // don't catch runtime_error here - let it propogate up to next level
  
  cs.defaultValueUnsignedInt("uin", 0);
  unsigned int uin = cs.getValueUnsignedInt("uin");

  ContactRef c;
  if (self) c = icqclient.getSelfContact();
  else {
    if (uin != 0)
      c = ContactRef( new ICQ2000::Contact(uin) ); // construct a real contact
    else
      c = ContactRef( new ICQ2000::Contact() );    // construct a virtual contact
  }

  // only needed for backward compatibility
  // (history_file settingsentry only exists for v >= 0.2.2)
  if (c->isICQContact()) {

    /* for 'real' contacts, we name the history file fixed to something
       sensible (user might want to grep it, etc..) */
    ostringstream historyfile;
    historyfile << uin << ".history";
    cs.defaultValueString("history_file", historyfile.str() );

  } else {

    /* only make up the default when it actually is defaulting
       - as the get_unique_historyname uses mkstemp so will create the file
       no matter what */
    if ( cs.getValueString("history_file").size() == 0 )
      cs.defaultValueString("history_file", get_unique_historyname());

  }

  cs.defaultValueUnsignedChar("age", 0, 0, 150);
  cs.defaultValueUnsignedChar("sex", 0, 0, 2);
  cs.defaultValueUnsignedShort("birth_year", 0, 1900, 2100);
  cs.defaultValueUnsignedChar("birth_month", 0, 0, 12);
  cs.defaultValueUnsignedChar("birth_day", 0, 0, 31);

  string alias = cs.getValueString("alias");
  c->setAlias(alias);
  c->setMobileNo(cs.getValueString("mobile_no"));
  c->setFirstName(cs.getValueString("firstname"));
  c->setLastName(cs.getValueString("lastname"));
  c->setEmail(cs.getValueString("email"));
	
  // Main Home Info
  ICQ2000::Contact::MainHomeInfo& mhi = c->getMainHomeInfo();
  mhi.city = cs.getValueString("city");
  mhi.state = cs.getValueString("state");
  mhi.phone = cs.getValueString("phone");
  mhi.fax = cs.getValueString("fax");
  mhi.street = cs.getValueString("street");
  mhi.zip = cs.getValueString("zip");
  mhi.country = cs.getValueUnsignedShort("country");
  mhi.timezone = cs.getValueUnsignedChar("gmt");
	
  // Homepage Info
  ICQ2000::Contact::HomepageInfo& hpi = c->getHomepageInfo();
  hpi.age = cs.getValueUnsignedChar("age");
  hpi.sex = cs.getValueUnsignedChar("sex");
  hpi.homepage = cs.getValueString("homepage");
  hpi.birth_year = cs.getValueUnsignedShort("birth_year");
  hpi.birth_month = cs.getValueUnsignedChar("birth_month");
  hpi.birth_day = cs.getValueUnsignedChar("birth_day");
  hpi.lang1 = cs.getValueUnsignedChar("lang1");
  hpi.lang2 = cs.getValueUnsignedChar("lang2");
  hpi.lang3 = cs.getValueUnsignedChar("lang3");

  // About Info
  c->setAboutInfo( cs.getValueString("about") );
	
  // Status
  c->set_signon_time( cs.getValueUnsignedInt("last_signon_time") );
  c->set_last_online_time( cs.getValueUnsignedInt("last_seen_online_time") );
  c->set_last_status_change_time( cs.getValueUnsignedInt("last_status_change_time") );
  c->set_last_message_time( cs.getValueUnsignedInt("last_message_time") );
  c->set_last_away_msg_check_time( cs.getValueUnsignedInt("last_away_msg_check_time") );

  if (!self) {
    m_settingsmap[c->getUIN()] = s;
    m_histmap[c->getUIN()] = new History( cs.getValueString("history_file") );
    icqclient.addContact(c);
  }
}

void IckleClient::loadSelfContact()
{
  try {
    loadContact( BASE_DIR + "self.user", true );
  } catch(runtime_error& e) {
    // ignore
  }
}

void IckleClient::check_pid_file(){

  fstream pidfile;
  pid_t pid;

  if (!mkdir_BASE_DIR()) return;

  pidfile.open(PID_FILENAME.c_str(), std::ios::in);

  if (pidfile.is_open()) {
    pidfile >> pid;
    if (getpid() == pid || (kill(pid, 0) == -1 && errno == ESRCH)) {
      cerr << "ickle left behind a stale lockfile (" << PID_FILENAME << ")" << endl;
    } else {
      gui.already_running_prompt( PID_FILENAME, pid );
      exit(1);
    }
  }

  pidfile.close();
  pidfile.open(PID_FILENAME.c_str(), std::ios::out);

  if (!pidfile.is_open()) {
    cerr << "Could not create pid_file (" << PID_FILENAME << ")" << endl;
  } else {
    pid = getpid();
    pidfile << pid;
    pidfile.close();
  }

}
