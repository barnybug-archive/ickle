/* $Id: IckleClient.cpp,v 1.132 2004-07-03 16:40:25 cborni Exp $
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
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#include <stdexcept>
#include <fstream>
#include <signal.h>

#include "ickle.h"
#include "ucompose.h"
#include "main.h"
#include "Icons.h"
#include "Dir.h"
#include "EventSubstituter.h"
#include "WizardDialog.h"
#include "utils.h"
#include "UserInfoHelpers.h"
#include "Translator.h"

#include <libicq2000/Client.h>

#include <time.h>

using std::string;
using std::istringstream;
using std::fstream;
using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::runtime_error;

using ICQ2000::ContactRef;

IckleClient::IckleClient(int argc, char* argv[])
  : m_message_queue(),
    m_event_system(m_message_queue),
    gui( m_message_queue, m_histmap ),
    status(ICQ2000::STATUS_OFFLINE),
    m_loading(true)
#ifdef CONTROL_SOCKET
    , ctrl(*this)
#endif
{
  // process command line parameters
  processCommandLine(argc,argv);
}

void IckleClient::init()
{
#ifdef CONTROL_SOCKET
  // initialize control socket
  ctrl.init();
#endif

  // let us know when the gui is destroyed
  gui.signal_destroy().connect(SigC::slot(*this,&IckleClient::quit));
  gui.signal_exit().connect(SigC::slot(*this,&IckleClient::exit_cb));

  // set up libICQ2000 Callbacks
  // -- callbacks into IckleClient

  icqclient.connected.connect(this,&IckleClient::connected_cb);
  icqclient.disconnected.connect(this,&IckleClient::disconnected_cb);
  icqclient.logger.connect(this,&IckleClient::logger_cb);
  icqclient.contactlist.connect(this,&IckleClient::contactlist_cb);
  icqclient.messaged.connect(this,&IckleClient::message_cb);
  icqclient.messageack.connect(this,&IckleClient::messageack_cb);
  icqclient.contact_status_change_signal.connect(&m_event_system,&EventSystem::status_change_cb);
  icqclient.socket.connect(this,&IckleClient::socket_cb);
  icqclient.want_auto_resp.connect(this,&IckleClient::want_auto_resp_cb);
  icqclient.filetransfer_incoming_signal.connect(this,&IckleClient::ft_incoming_cb);
  icqclient.filetransfer_update_signal.connect(this,&IckleClient::ft_update_cb);
  // message queue callbacks
  m_message_queue.added.connect(SigC::slot(*this,&IckleClient::queue_added_cb));
  m_message_queue.added.connect(SigC::slot(m_event_system, &EventSystem::queue_added_cb));

  // set up GUI callbacks
  gui.signal_save_settings().connect(SigC::slot(*this,&IckleClient::save_settings_cb));
  gui.signal_send_event().connect(SigC::slot(*this,&IckleClient::send_event_cb));
  gui.signal_restart_client.connect(SigC::slot(*this,&IckleClient::apply_and_restart) );

  gui.setDisplayTimes(true);

  loadSettings();
  loadSelfContact();

  gui.post_settings_loaded();

  // setup contact list
  loadContactList();

  g_translator.signal_contact_encoding_changed().connect( SigC::slot( *this, &IckleClient::contact_encoding_changed_cb ) );

  gui.show_all();

  ICQ2000::Status st = ICQ2000::Status(g_settings.getValueUnsignedInt("autoconnect"));
  // give gtk some time to breathe before we do an auto connect - dns
  // lookup will block which would prevent them seeing anything for as
  // long as that takes
  if (st != ICQ2000::STATUS_OFFLINE)
    Glib::signal_idle().connect( SigC::bind( SigC::slot( *this, &IckleClient::idle_connect_cb ), st, false ) );


  if( g_settings.getValueUnsignedInt("uin") == 0 )
  {
    WizardDialog wiz;
    if( wiz.run() ) // if we got an uin, automatically go online to allow the user to set his info
      Glib::signal_idle().connect( SigC::bind( SigC::slot( *this, &IckleClient::idle_connect_cb ), ICQ2000::STATUS_ONLINE, false ) );
  }

}

IckleClient::~IckleClient() {
  // free History objects
  for( map<unsigned int, History *>::iterator itr = m_histmap.begin(); itr != m_histmap.end(); ++itr )
    delete itr->second;
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

void IckleClient::processCommandLine(int argc, char* argv[])
{
  int i = 0;
  while ( ( i = getopt( argc, argv, "hb:" ) ) > 0) {

    switch(i) {
    case '?':
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

void IckleClient::usageInstructions(const char* progname)
{
  cout << String::ucompose(_("ickle version %1"), ICKLE_VERSION) << endl
       << String::ucompose(_("Usage: %1 [-h] [-b dir]"), progname) << endl << endl
       << Glib::ustring(_(" -h : the help screen you are seeing")) << endl
       << Glib::ustring(_(" -b : use a different configuration directory (~/.ickle/ is the default)")) << endl
       << endl;
}


void IckleClient::SignalLog(ICQ2000::LogEvent::LogType type, const string& msg)
{
  ICQ2000::LogEvent ev(type,msg);
  logger_cb(&ev);
}

void IckleClient::loadSettings()
{
  // load in settings
  try
  {
    g_settings.load(BASE_DIR + "ickle.conf");
  }
  catch (runtime_error& e)
  {
    SignalLog(ICQ2000::LogEvent::WARN,
	      String::ucompose( _("Couldn't open %1ickle.conf, using default settings\n"
				  "This is probably the first time you've run ickle.\n"), BASE_DIR ));
  }

  /*
   * Default settings & limits to numerical values
   */
  g_settings.defaultValueBool("away_autoposition", true);
  g_settings.defaultValueUnsignedShort("auto_away", 0, 0, 65535);
  g_settings.defaultValueUnsignedShort("auto_na", 0, 0, 65535);
  g_settings.defaultValueBool("auto_return", true);
  g_settings.defaultValueBool("auto_reconnect", true);
  g_settings.defaultValueUnsignedInt("reconnect_retries", 2, 0, 10);

  g_settings.defaultValueBool("window_status_icons", true);

  /* Event settings */
  g_settings.defaultValueUnsignedInt("event_repetition_threshold", 100);
  g_settings.defaultValueBool("event_execute_all", false);

  /* Default log settings */
  g_settings.defaultValueBool("log_to_console", true);
  g_settings.defaultValueBool("log_to_file", false);
  g_settings.defaultValueString("logfile", "messages.log");
  g_settings.defaultValueBool("log_info", false);
  g_settings.defaultValueBool("log_error", true);
  g_settings.defaultValueBool("log_warn", true);
  g_settings.defaultValueBool("log_packet", false);
  g_settings.defaultValueBool("log_directpacket", false);

  g_settings.defaultValueBool("log_window_info", false);
  g_settings.defaultValueBool("log_window_error", true);
  g_settings.defaultValueBool("log_window_warn", true);
  g_settings.defaultValueBool("log_window_packet", false);
  g_settings.defaultValueBool("log_window_directpacket", false);

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

  g_settings.defaultValueString("last_auto_response", _("I really enjoy the default away message of ickle %v") );
  g_settings.defaultValueBool("set_away_response_dialog", true);
  g_settings.defaultValueBool("set_away_response_timeout", true);
  g_settings.defaultValueBool("mouse_single_click", false);
  g_settings.defaultValueBool("mouse_check_away_click", true);
  g_settings.defaultValueUnsignedInt("sort_contact_list_column", 0, 0, 1000);
  g_settings.defaultValueBool("spell_check", true);
  g_settings.defaultValueString("spell_check_lang","");
  g_settings.defaultValueBool("initial_userinfo_done", false );

  g_settings.defaultValueUnsignedInt("no_autoresponses", 1, 1, 100);
  g_settings.defaultValueString("autoresponse_1_label", _("Away"));
  g_settings.defaultValueString("autoresponse_1_text", _("Hello %a, I'm away at the moment.") );

  g_settings.defaultValueBool("show_offline_contacts", true);

  g_settings.defaultValueString("encoding", "ISO-8859-1"); // this is the on-the-wire encoding

  // Set settings in library
  initLibrary();

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


  g_icons.setIcons( g_settings.getValueString("icons_dir") );

  m_retries = g_settings.getValueUnsignedChar("reconnect_retries");

  // Load up auto response in case autoconnect = away/na/etc.
  string auto_response = g_settings.getValueString("last_auto_response");
  gui.setAutoResponse(auto_response);

  if (g_settings.getValueBool("spell_check"))
    gui.spell_check_setup();

  // Load in Groups
  unsigned int no_groups = g_settings.getValueUnsignedInt("no_groups");
  for (unsigned int i = 1; i <= no_groups; i++)
  {
    std::string fetch_str  = Utils::format_string( "group_%d_label", i );
    std::string fetch_str2 = Utils::format_string( "group_%d_id", i );

    std::string label = g_settings.getValueString(fetch_str);
    unsigned short id = g_settings.getValueUnsignedShort(fetch_str2);

    icqclient.getContactTree().add_group( label, id );
  }

  m_loading = false;
}


void IckleClient::initLibrary ()
{
  icqclient.setUIN(g_settings.getValueUnsignedInt("uin"));
  icqclient.setPassword(g_settings.getValueString("password"));
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

  if (g_settings.getValueBool("network_use_portrange")) {
    // use a port range
    icqclient.setUsePortRange( true );
    icqclient.setPortRangeLowerBound( g_settings.getValueUnsignedShort("network_lower_bind_port") );
    icqclient.setPortRangeUpperBound( g_settings.getValueUnsignedShort("network_upper_bind_port") );
  }

  icqclient.setAcceptInDC( g_settings.getValueBool("network_in_dc") );
  icqclient.setUseOutDC( g_settings.getValueBool("network_out_dc") );

  // enable SBL support
  icqclient.fetchServerBasedContactList();

  // set Translator
  icqclient.set_translator( new IckleTranslatorProxy( g_translator ) );

  // --
}

// stop the library, reload the settings and reconnect to the former state
void IckleClient::apply_and_restart ()
{
  ICQ2000::Status oldstat = icqclient.getStatus();
  if (oldstat != ICQ2000::STATUS_OFFLINE )
    icqclient.setStatus(ICQ2000::STATUS_OFFLINE);

  initLibrary();
  icqclient.setStatus(oldstat);
}

void IckleClient::saveSettings()
{
  int width, height, x, y;

  gui.get_window()->get_root_origin(x, y);
  gui.get_window()->get_size(width, height);
  g_settings.setValue( "geometry_x", x );
  g_settings.setValue( "geometry_y", y );
  g_settings.setValue( "geometry_width", width );
  g_settings.setValue( "geometry_height", height );

  if (!mkdir_BASE_DIR()) return;

  string ickle_conf = BASE_DIR + "ickle.conf";

  // set umask to secure value, so that if ickle.conf doesn't exist, and is created it will be safe.
  mode_t old_umask = umask(0077);

  try
  {
    g_settings.save(ickle_conf);
  }
  catch(runtime_error& e)
  {
    SignalLog(ICQ2000::LogEvent::ERROR, String::ucompose( _("Couldn't save %1ickle.conf"), BASE_DIR ));
  }

  umask(old_umask);

  // ensure permissions on ickle.conf are secure
  if ( chmod( ickle_conf.c_str(), S_IRUSR | S_IWUSR ) == -1 )
  {
    SignalLog(ICQ2000::LogEvent::ERROR,
	      String::ucompose( _("The permissions on %1 couldn't be set to 0600. Your ICQ password is vulnerable!"),
				ickle_conf ) );
  }

  // save self-contact
  saveSelfContact();

  // save contact-specific settings
  for( map<unsigned int, string>::iterator itr = m_settingsmap.begin(); itr != m_settingsmap.end(); ++itr ) {
    ContactRef c = icqclient.getContact( itr->first );
    if (c.get() != NULL) saveContact( c, itr->second, false );
  }

}

void IckleClient::exit_cb()
{
  /*
   * These both need to be done while the widget
   * still exists, in IckleClient::quit is too late
   */
  saveSettings();
  icqclient.setStatus(ICQ2000::STATUS_OFFLINE);

  Gtk::Main::quit();
}

void IckleClient::quit()
{
  // remove the ickle.pid file
  if (!PID_FILENAME.empty())
    unlink(PID_FILENAME.c_str());

#ifdef CONTROL_SOCKET
  ctrl.quit();
#endif
}

void IckleClient::connected_cb(ICQ2000::ConnectedEvent *)
{
  /* the library needs to be polled regularly
   * to ensure timeouts are respected and the server is pinged every minute
   * A 5 second interval is sensible for this
   */
  poll_server_cnt = Glib::signal_timeout().connect( SigC::slot( *this, &IckleClient::poll_server_cb ), 5000 );
  m_retries = g_settings.getValueUnsignedChar("reconnect_retries");

  if( !g_settings.getValueBool("initial_userinfo_done") ) {
    gui.my_user_info_cb();
    g_settings.setValue("initial_userinfo_done", true );
  }
}

void IckleClient::disconnected_cb(ICQ2000::DisconnectedEvent *c) {
  if (c->getReason() == ICQ2000::DisconnectedEvent::REQUESTED) {
    SignalLog(ICQ2000::LogEvent::INFO, _("Disconnected as requested") );
  } else {
    string reason;
    switch(c->getReason()) {
    case ICQ2000::DisconnectedEvent::FAILED_LOWLEVEL:
      reason = _("Socket problems");
      break;
    case ICQ2000::DisconnectedEvent::FAILED_BADUSERNAME:
      reason = _("Bad Username");
      break;
    case ICQ2000::DisconnectedEvent::FAILED_TURBOING:
      reason = _("Turboing");
      break;
    case ICQ2000::DisconnectedEvent::FAILED_BADPASSWORD:
      reason = _("Bad Password");
      break;
    case ICQ2000::DisconnectedEvent::FAILED_MISMATCH_PASSWD:
      reason = _("Username and Password did not match");
      break;
    case ICQ2000::DisconnectedEvent::FAILED_UNKNOWN:
      reason = _("Unknown");
      break;
    case ICQ2000::DisconnectedEvent::FAILED_DUALLOGIN:
      reason = _("Dual login, disconnected");
      break;
    default:
      reason = _("Unknown");
      break;
    }
    SignalLog(ICQ2000::LogEvent::ERROR,
	      String::ucompose( _("Problem connecting: %1"), reason ) );
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
    default:
      break;
    }

  if (m_retries > 0) {
    --m_retries;
    Glib::signal_idle().connect( SigC::bind( SigC::slot( *this, &IckleClient::idle_connect_cb ),
					     icqclient.getStatusWanted(),
					     icqclient.getInvisibleWanted() ) );
  }
  else {
    m_retries = g_settings.getValueUnsignedChar("reconnect_retries"); // reset m_retries

    if( c->getReason() == ICQ2000::DisconnectedEvent::FAILED_LOWLEVEL )
      gui.disconnect_lowlevel_prompt(m_retries);
    else if( c->getReason() == ICQ2000::DisconnectedEvent::FAILED_UNKNOWN )
      gui.disconnect_unknown_prompt(m_retries);
  }
}

bool IckleClient::idle_connect_cb(ICQ2000::Status s, bool inv)
{
  icqclient.setStatus( s, inv );
  return false;
}

void IckleClient::logger_file_cb(const string& msg)
{
  string log_file = BASE_DIR + g_settings.getValueString("logfile");

  // set umask to secure value, so that if ickle.conf doesn't exist, and is created it will be safe.
  mode_t old_umask = umask(0077);

  ofstream of(log_file.c_str(), std::ios::out | std::ios::app);
  if (of)
    of << msg;
  of.close();

  umask(old_umask);

  // ensure permissions on log file are secure
  if ( chmod( log_file.c_str(), S_IRUSR | S_IWUSR ) == -1 )
  {
    cout << String::ucompose( _("The permissions on %1 couldn't be set to 0600. Your ICQ password may be logged in this file, so it could be vulnerable!"),
			      log_file ) << endl;
  }
}

void IckleClient::logger_cb(ICQ2000::LogEvent *c)
{
  bool log_to_console = g_settings.getValueBool("log_to_console");
  bool log_to_file = g_settings.getValueBool("log_to_file");
  if (!log_to_console && !log_to_file) return;

  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char time_str[256];
  strftime(time_str, 255, "[%H:%M:%S]", tm);

  if ( log_to_console && log_to_file )
  {
    logger_file_cb( String::ucompose( "%1 %2\n", time_str, c->getMessage() ) );
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

  if ( !log_to_console && log_to_file )
  {
    logger_file_cb( String::ucompose( "%1 %2\n", time_str, c->getMessage() ) );
  }

  if (log_to_console) {
    cout << time_str << " ";
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
    cout << Utils::locale_from_utf8_with_fallback(c->getMessage())
            // console output should be in current locale
	 << endl;
    cout << "[39m";
  }

}

string IckleClient::get_unique_historyname() throw (runtime_error)
{
  char tmpl[255];
  int fd;

  snprintf( tmpl, sizeof(tmpl), "%smobilehistory.XXXXXX", CONTACT_DIR.c_str() );
  if( (fd = mkstemp( tmpl ) ) == -1 )
    throw( runtime_error( _("Could not generate temporary filename for history") ) );
  close(fd);
  return string ( tmpl + CONTACT_DIR.size() );
}

bool IckleClient::socket_select_cb(Glib::IOCondition cond, int source) {
  icqclient.socket_cb(source,
		      (ICQ2000::SocketEvent::Mode)
		      ((cond & Glib::IO_IN ? ICQ2000::SocketEvent::READ : 0) |
		       (cond & Glib::IO_OUT ? ICQ2000::SocketEvent::WRITE : 0) |
		       (cond & Glib::IO_ERR ? ICQ2000::SocketEvent::EXCEPTION : 0)));
  return true;
}

bool IckleClient::poll_server_cb()
{
  icqclient.Poll();
  return true;
}

void IckleClient::send_event_cb(ICQ2000::MessageEvent *ev)
{
  icqclient.SendEvent(ev);
}

void IckleClient::messageack_cb(ICQ2000::MessageEvent *ev)
{
  if (ev->isFinished() && ev->isDelivered()
      && ev->getType() != ICQ2000::MessageEvent::AwayMessage)
  {
    unsigned int uin = ev->getContact()->getUIN();
    if (m_histmap.count(uin) == 0)
    {
      SignalLog(ICQ2000::LogEvent::ERROR,
		String::ucompose( _("No history object for contact: %1"), uin ) );
    } else {
      History *h = m_histmap[ev->getContact()->getUIN()];
      try {
	h->log(ev, false);
      } catch(runtime_error& e) {
	// thrown when the history file can't be opened
	SignalLog(ICQ2000::LogEvent::WARN, e.what());
      }
    }

  }

}

void IckleClient::socket_cb(ICQ2000::SocketEvent *ev) {
  
  if (dynamic_cast<ICQ2000::AddSocketHandleEvent*>(ev) != NULL) {
    ICQ2000::AddSocketHandleEvent *cev = dynamic_cast<ICQ2000::AddSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    if (m_sockets.count(fd) > 0) {
      // uh oh..
      SignalLog(ICQ2000::LogEvent::ERROR, _("Problem: file descriptor already connected") );
      m_sockets[fd].disconnect();
      m_sockets.erase(fd);
    }

    m_sockets[fd] = Glib::signal_io().connect(SigC::bind(SigC::slot(*this, &IckleClient::socket_select_cb), fd), fd,
					      (Glib::IOCondition)((cev->isRead()      ? Glib::IO_IN  : 0) |
								  (cev->isWrite()     ? Glib::IO_OUT : 0) |
								  (cev->isException() ? Glib::IO_ERR : 0)));

  } else if (dynamic_cast<ICQ2000::RemoveSocketHandleEvent*>(ev) != NULL) {
    ICQ2000::RemoveSocketHandleEvent *cev = dynamic_cast<ICQ2000::RemoveSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    if (m_sockets.count(fd) == 0) {
      SignalLog(ICQ2000::LogEvent::ERROR, _("Problem: file descriptor not connected") );
    } else {
      m_sockets[fd].disconnect();
      m_sockets.erase(fd);
    }
  }
}


void IckleClient::ft_incoming_cb(ICQ2000::FileTransferEvent *ev)
{
  message_cb(ev);
}

void IckleClient::ft_update_cb(ICQ2000::FileTransferEvent *ev)
{
  // need to update Events in the MessageQueue if they are cancelled
  // whilst still queueing
  MessageQueue::iterator curr = m_message_queue.begin();
  while (curr != m_message_queue.end()) {
    if ((*curr)->getServiceType() == MessageEvent::ICQ)
    {
      ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(*curr);
      if (icq->getICQMessageType() == ICQMessageEvent::FileTransfer) {
	FileTransferICQMessageEvent *ft = static_cast<FileTransferICQMessageEvent*>(icq);
	if (ft->getEvent() == ev) ft->setCancelled(true);
      }
    }
    ++curr;
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
  ICQ2000::ContactTree& ct = icqclient.getContactTree();
  if (!ct.exists(ev->getContact()->getUIN())) {
    // add into 'New' group, create group if necessarily
    ICQ2000::ContactTree::Group *gp = NULL;
    ICQ2000::ContactTree::iterator curr = ct.begin();
    while (curr != ct.end()) {
      if ((*curr).get_label() == _("New") ) {
	gp = &(*curr);
	break;
      }
      ++curr;
    }
    if (gp == NULL) gp = &(ct.add_group( _("New") ));
    
    gp->add(ev->getContact());
    icqclient.fetchDetailContactInfo(ev->getContact());
  }

  // convert to a client event type and add to queue
  MessageEvent *ickle_ev = convert_libicq2000_event(ev);
  if (ickle_ev == NULL) {
    SignalLog(ICQ2000::LogEvent::INFO, _("Unhandled libicq2000 event") );
    return;
  }
  
  m_message_queue.add_to_queue(ickle_ev);

  unsigned int uin = ev->getContact()->getUIN();
  if (m_histmap.count(uin) == 0)
  {
      SignalLog(ICQ2000::LogEvent::ERROR,
		String::ucompose( _("No history object for contact: %1"), uin ) );
  } else {
    History *h = m_histmap[ev->getContact()->getUIN()];
    try {
      h->log(ev, true);
    } catch(runtime_error& e) {
      // thrown when the history file can't be opened
      SignalLog(ICQ2000::LogEvent::WARN, e.what());
    }
  }
  
}

void IckleClient::queue_added_cb(MessageEvent *)
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
  case ICQ2000::MessageEvent::WebPager:
  {
    ICQ2000::WebPagerEvent *cev = static_cast<ICQ2000::WebPagerEvent*>(ev);
    ret = new WebPagerICQMessageEvent( ev->getTime(), c, cev->getMessage() );
    break;
  }
  case ICQ2000::MessageEvent::UserAdd:
  {
    ret = new UserAddICQMessageEvent( ev->getTime(), c );
    break;
  }
  case ICQ2000::MessageEvent::FileTransfer:
  {
    ICQ2000::FileTransferEvent *fev = static_cast<ICQ2000::FileTransferEvent*>(ev);
    ret = new FileTransferICQMessageEvent( ev->getTime(), c, fev->getMessage(),
					   fev->getDescription(), fev->getSize(), fev);
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

void IckleClient::want_auto_resp_cb(ICQ2000::ICQMessageEvent *ev)
{
  ICQ2000::ContactRef c = ev->getContact();
  EventSubstituter evs(m_message_queue, c);
  evs << g_settings.getValueString( "last_auto_response" );
  ev->setAwayMessage(evs.str());
}

bool IckleClient::mkdir_BASE_DIR()
{
  if ( mkdir( BASE_DIR.c_str(), 0700 ) == -1 && errno != EEXIST )
  {
    SignalLog(ICQ2000::LogEvent::ERROR,
	      String::ucompose( _("mkdir %1 failed: %2"),
				BASE_DIR,
				strerror(errno) ) );
    return false;
  }
  return true;
}

std::string IckleClient::get_unique_contact_user_filename(ICQ2000::ContactRef& c)
{
  std::string filename = Utils::format_string( "%s%d.user", CONTACT_DIR.c_str(), c->getUIN() );

  // ensure uniqueness
  int n = 0;
  struct stat fs;
  
  while ( stat( filename.c_str(), &fs ) == 0 )
  {
    n++;
    filename = Utils::format_string("%s%d-%d.user", CONTACT_DIR.c_str(), c->getUIN(), n);
  }

  return filename;
}

std::string IckleClient::get_contact_history_filename(ICQ2000::ContactRef& c)
{
  return Utils::format_string( "%d.history", c->getUIN() );
}

void IckleClient::contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  if (ev->getType() == ICQ2000::ContactListEvent::UserAdded) {
    ICQ2000::UserAddedEvent *cev = static_cast<ICQ2000::UserAddedEvent*>(ev);
    ContactRef c = cev->getContact();
  
    if (m_settingsmap.count(c->getUIN()) > 0)
      return;

    string filename = get_unique_contact_user_filename(c);

    m_settingsmap[c->getUIN()] = filename;

    if( c->isICQContact() )
    {
      m_histmap[c->getUIN()] = new History( get_contact_history_filename(c) );
    }
    else
    {
      m_histmap[c->getUIN()] = new History( get_unique_historyname() );
    }

    if ( !mkdir_BASE_DIR() ) return;

    if ( mkdir( CONTACT_DIR.c_str(), 0700 ) == -1 && errno != EEXIST )
    {
      SignalLog(ICQ2000::LogEvent::ERROR,
		String::ucompose( _("mkdir %1 failed: %2"),
				  CONTACT_DIR,
				  strerror(errno) ) );
      return;
    }
    
    saveContact( c, m_settingsmap[c->getUIN()], false );
    
  } else if (ev->getType() == ICQ2000::ContactListEvent::UserRemoved) {
    ICQ2000::UserRemovedEvent *cev = static_cast<ICQ2000::UserRemovedEvent*>(ev);
    ContactRef c = cev->getContact();

    // delete .user file for this contact
    unlink( m_settingsmap[c->getUIN()].c_str() );

    // remove history file for mobile users as well, we will not be able to correctly
    // reuse this history file if the same user is added anyway
    if( !c->isICQContact() )
      unlink( string(CONTACT_DIR + m_histmap[c->getUIN()]->getFilename()).c_str() );

    m_histmap.erase(c->getUIN());
    m_settingsmap.erase(c->getUIN());

  } else if (ev->getType() == ICQ2000::ContactListEvent::UserRelocated)
  {
    ICQ2000::UserRelocatedEvent *cev = static_cast<ICQ2000::UserRelocatedEvent*>(ev);
    ContactRef c = cev->getContact();
    saveContact( c, m_settingsmap[c->getUIN()], false );
  }
  else if (!m_loading && (ev->getType() == ICQ2000::ContactListEvent::GroupAdded
	     || ev->getType() == ICQ2000::ContactListEvent::GroupChange))
  {
    // update g_settings and save them
    update_group_settings();
    //saveSettings();
  }
  else if (ev->getType() == ICQ2000::ContactListEvent::GroupRemoved)
  {
    ICQ2000::GroupRemovedEvent *cev = static_cast<ICQ2000::GroupRemovedEvent*>(ev);
    ICQ2000::ContactTree& ct = icqclient.getContactTree();

    // clear first
    unsigned int num = g_settings.getValueUnsignedInt("no_groups");
    for (unsigned int i = 1; i <= num; ++i)
    {
      std::string fetch_str  = Utils::format_string( "group_%d_label", i );
      std::string fetch_str2 = Utils::format_string( "group_%d_id", i );
      
      g_settings.removeValue( fetch_str );
      g_settings.removeValue( fetch_str2 );
    }
    
    /* group won't have been removed yet */

    unsigned int j = 0;
    ICQ2000::ContactTree::iterator curr = ct.begin();
    while (curr != ct.end())
    {
      if (curr->get_id() != cev->get_group().get_id())
      {
	++j;
	
	std::string fetch_str  = Utils::format_string( "group_%d_label", j );
	std::string fetch_str2 = Utils::format_string( "group_%d_id",    j );

	g_settings.setValue(fetch_str,  (*curr).get_label());
	g_settings.setValue(fetch_str2, (*curr).get_id());
      }
      ++curr;
    }

    g_settings.setValue("no_groups", j );

    saveSettings();
  }
    
}

void IckleClient::update_group_settings()
{
  // save Groups
  ICQ2000::ContactTree& ct = icqclient.getContactTree();
  g_settings.setValue("no_groups", ct.group_size() );
  unsigned int i = 1;

  ICQ2000::ContactTree::iterator curr = ct.begin();
  while (curr != ct.end())
  {
    std::string fetch_str  = Utils::format_string( "group_%d_label", i );
    std::string fetch_str2 = Utils::format_string( "group_%d_id", i );

    g_settings.setValue(fetch_str,  (*curr).get_label());
    g_settings.setValue(fetch_str2, (*curr).get_id());
    
    ++i;
    ++curr;
  }
}

void IckleClient::save_settings_cb()
{
  saveSettings();
}

void IckleClient::saveContact(ContactRef c, const string& s, bool self)
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

  // Character-set encoding
  if ( g_translator.is_contact_encoding(c) )
  {
    user.setValue( "encoding", g_translator.get_contact_encoding(c) );
  }
  else
  {
    user.removeValue( "encoding" );
  }

  if (!self) {
    // save history mapping
    user.setValue( "history_file", m_histmap[c->getUIN()]->getFilename() );

    ICQ2000::ContactTree& ct = icqclient.getContactTree();
    user.setValue( "group_id", ct.lookup_group_containing_contact( c ).get_id() );
  }

  try {
    user.save(s);
  } catch(runtime_error& e) {
    SignalLog(ICQ2000::LogEvent::ERROR, e.what());
  }
}

void IckleClient::saveSelfContact()
{
  saveContact( icqclient.getSelfContact(), BASE_DIR + "self.user", true );
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
  if (c->isICQContact())
  {
    /* for 'real' contacts, we name the history file fixed to something
       sensible (user might want to grep it, etc..) */
    cs.defaultValueString("history_file", get_contact_history_filename(c) );
  }
  else
  {
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
  mhi.city     = cs.getValueString("city");
  mhi.state    = cs.getValueString("state");
  mhi.phone    = cs.getValueString("phone");
  mhi.fax      = cs.getValueString("fax");
  mhi.street   = cs.getValueString("street");
  mhi.zip      = cs.getValueString("zip");
  mhi.country  = ICQ2000::Country(cs.getValueUnsignedShort("country"));
  mhi.timezone = ICQ2000::Timezone(cs.getValueUnsignedChar("gmt"));
	
  // Homepage Info
  ICQ2000::Contact::HomepageInfo& hpi = c->getHomepageInfo();
  hpi.age         = cs.getValueUnsignedChar("age");
  hpi.sex         = ICQ2000::Sex(cs.getValueUnsignedChar("sex"));
  hpi.homepage    = cs.getValueString("homepage");
  hpi.birth_year  = cs.getValueUnsignedShort("birth_year");
  hpi.birth_month = cs.getValueUnsignedChar("birth_month");
  hpi.birth_day   = cs.getValueUnsignedChar("birth_day");
  hpi.lang1       = ICQ2000::Language(cs.getValueUnsignedChar("lang1"));
  hpi.lang2       = ICQ2000::Language(cs.getValueUnsignedChar("lang2"));
  hpi.lang3       = ICQ2000::Language(cs.getValueUnsignedChar("lang3"));

  // About Info
  c->setAboutInfo( cs.getValueString("about") );
	
  // Status
  c->set_signon_time( cs.getValueUnsignedInt("last_signon_time") );
  c->set_last_online_time( cs.getValueUnsignedInt("last_seen_online_time") );
  c->set_last_status_change_time( cs.getValueUnsignedInt("last_status_change_time") );
  c->set_last_message_time( cs.getValueUnsignedInt("last_message_time") );
  c->set_last_away_msg_check_time( cs.getValueUnsignedInt("last_away_msg_check_time") );

  // Character set
  if (!self)
  {
    if ( cs.getValueString("encoding").size() )
    {
      g_translator.set_contact_encoding(c, cs.getValueString("encoding") );
    }
  }

  if (!self) {
    m_settingsmap[c->getUIN()] = s;
    m_histmap[c->getUIN()] = new History( cs.getValueString("history_file") );

    ICQ2000::ContactTree& ct = icqclient.getContactTree();
    ICQ2000::ContactTree::Group *gp = NULL;

    if (cs.exists("group_id") &&
	ct.exists_group(cs.getValueUnsignedShort("group_id"))) {
      gp = &(ct.lookup_group(cs.getValueUnsignedShort("group_id")));
    } else {
      // add into 'New' group, create group if necessarily
      ICQ2000::ContactTree::iterator curr = ct.begin();
      while (curr != ct.end()) {
	if ((*curr).get_label() == _("New") )
	{
	  gp = &(*curr);
	  break;
	}
	++curr;
      }
      if (gp == NULL) gp = &(ct.add_group( _("New") ));
    }
    
    gp->add(c);
  }
}

void IckleClient::loadSelfContact()
{
  try
  {
    loadContact( BASE_DIR + "self.user", true );
  }
  catch(runtime_error& e)
  {
    // ignore
  }
}

bool IckleClient::check_pid_file()
{
  std::ifstream ipidfile;
  pid_t pid;

  if (!mkdir_BASE_DIR()) return true;

  ipidfile.open(PID_FILENAME.c_str(), std::ios::in);

  if (ipidfile.is_open())
  {
    ipidfile >> pid;
    if (getpid() == pid || (kill(pid, 0) == -1 && errno == ESRCH))
    {
      cerr << Utils::console(String::ucompose( _("ickle left behind a stale lockfile (%1)"), PID_FILENAME ) )
	   << endl;
    }
    else
    {
      gui.already_running_prompt( PID_FILENAME, pid );
      return false;
    }
  }

  ipidfile.close();

  std::ofstream opidfile;
  opidfile.open(PID_FILENAME.c_str(), std::ios::out);

  if (!opidfile.is_open())
  {
    cerr << Utils::console(String::ucompose( _("Could not create pid_file (%1)"), PID_FILENAME ) )
	 << endl;
  }
  else
  {
    pid = getpid();
    opidfile << pid;
    opidfile.close();
  }

  return true;
}

void IckleClient::contact_encoding_changed_cb(unsigned int uin)
{
  ICQ2000::ContactRef c = icqclient.getContactTree().lookup_uin(uin);
  saveContact( c, m_settingsmap[uin], false );
}
