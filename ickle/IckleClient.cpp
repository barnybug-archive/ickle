/*
 * IckleClient.cpp
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

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#include <fstream>
#include "sstream_fix.h"

#include "main.h"
#include "Icons.h"
#include "Dir.h"

#include "Client.h"

#include <time.h>

using std::ostringstream;
using std::istringstream;
using std::ofstream;
using std::cout;
using std::endl;

IckleClient::IckleClient(int argc, char* argv[])
  : gui(),
    status(STATUS_OFFLINE)
{
  // process command line parameters
  processCommandLine(argc,argv);

#ifdef GNOME_ICKLE
  // initialize GNOME applet
  applet.init(argc, argv, gui);
  applet.user_popup.connect( slot( this,&IckleClient::user_popup_cb ) );
#endif

  // let us know when the gui is destroyed
  gui.destroy.connect(slot(this,&IckleClient::quit));
  gui.delete_event.connect(slot(this,&IckleClient::close_cb));
  
  // set up libICQ2000 Callbacks
  // -- callbacks into IckleClient

  icqclient.connected.connect(slot(this,&IckleClient::connected_cb));
  icqclient.disconnected.connect(slot(this,&IckleClient::disconnected_cb));
  icqclient.logger.connect(slot(this,&IckleClient::logger_cb));
  icqclient.contactlist.connect(slot(this,&IckleClient::contactlist_cb));
  icqclient.messaged.connect(slot(this,&IckleClient::message_cb));
  icqclient.messageack.connect(slot(this,&IckleClient::messageack_cb));
  icqclient.socket.connect(slot(this,&IckleClient::socket_cb));

  // set up GUI callbacks
  gui.settings_changed.connect(slot(this,&IckleClient::settings_changed_cb));
  gui.fetch.connect( slot( this, &IckleClient::fetch_cb ) );
  gui.user_popup.connect( slot( this,&IckleClient::user_popup_cb ) );
  gui.getContactListView()->user_popup.connect( slot( this,&IckleClient::user_popup_cb ) );
  gui.getContactListView()->userinfo.connect( slot( this, &IckleClient::userinfo_cb ) );

  gui.send_event.connect(slot(this,&IckleClient::send_event_cb));
  gui.add_user.connect(slot(this,&IckleClient::add_user_cb));
  gui.add_mobile_user.connect(slot(this,&IckleClient::add_mobile_user_cb));

  gui.setDisplayTimes(true);

  loadSettings();

  // setup contact list
  loadContactList();

  gui.show_all();

  Status st = Status(g_settings.getValueUnsignedInt("autoconnect"));
  if (st != STATUS_OFFLINE) icqclient.setStatus(st);
}

IckleClient::~IckleClient() {
  g_icons.FreeIcons();

  // free History objects
  for( hash_map<unsigned int, History *>::iterator itr = m_histmap.begin(); itr != m_histmap.end(); ++itr )
    delete itr->second;
}

void IckleClient::loadContactList() {
  Dir dir;
  dir.list( CONTACT_DIR + "*.user" );

  Dir::iterator dirit = dir.begin();
  while( dirit != dir.end() ) {
    
    Settings cs;
    if (cs.load(*dirit)) {
      cs.defaultValueUnsignedInt("uin", 0);
      unsigned int uin = cs.getValueUnsignedInt("uin");
      if (uin != 0) {
	Contact c(uin);
	
	cs.defaultValueUnsignedChar("age", 0, 0, 150);
	cs.defaultValueUnsignedChar("sex", 0, 0, 2);
	cs.defaultValueUnsignedShort("birth_year", 0, 1900, 2100);
	cs.defaultValueUnsignedChar("birth_month", 0, 0, 12);
	cs.defaultValueUnsignedChar("birth_day", 0, 0, 31);

	string alias = cs.getValueString("alias");
	c.setAlias(alias);
	c.setMobileNo(cs.getValueString("mobile_no"));
	c.setFirstName(cs.getValueString("firstname"));
	c.setLastName(cs.getValueString("lastname"));
	c.setEmail(cs.getValueString("email"));
	
	// Main Home Info
	MainHomeInfo& mhi = c.getMainHomeInfo();
	mhi.city = cs.getValueString("city");
	mhi.state = cs.getValueString("state");
	mhi.phone = cs.getValueString("phone");
	mhi.fax = cs.getValueString("fax");
	mhi.street = cs.getValueString("street");
	mhi.zip = cs.getValueString("zip");
	mhi.country = cs.getValueUnsignedShort("country");
	mhi.gmt = cs.getValueUnsignedChar("gmt");
	
	// Homepage Info
	HomepageInfo& hpi = c.getHomepageInfo();
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
	c.setAboutInfo( cs.getValueString("about") );
	
	m_fmap[c.getUIN()] = *dirit;
	m_histmap[c.getUIN()] = new History( c.getUIN() );
	icqclient.addContact(c);
      } else {
	Contact c( cs.getValueString("alias"), cs.getValueString("mobile_no") );
	m_fmap[c.getUIN()] = *dirit;
	m_histmap[c.getUIN()] = new History( c.getUIN() );
	icqclient.addContact(c);
      }
    }

    ++dirit;

  }

}

void IckleClient::processCommandLine(int argc, char* argv[]) {
  int i = 0;
  while ( ( i = getopt( argc, argv, "hb:" ) ) > 0) {

    switch(i) {
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
}

void IckleClient::usageInstructions(const char* progname) {
  cout << "ickle version 0.2.0" << endl
       << "Usage: " << progname << " [-h] [-b dir]" << endl << endl
       << " -h : the help screen you are seeing" << endl
       << " -b : use a different configuration directory (~/.ickle/ is the default)" << endl
       << endl;
}


void IckleClient::SignalLog(LogEvent::LogType type, const string& msg) {
  LogEvent ev(type,msg);
  logger_cb(&ev);
}
  
void IckleClient::loadSettings() {
  // load in settings
  if (!g_settings.load(BASE_DIR + "ickle.conf")) {
    ostringstream ostr;
    ostr << "Couldn't open " << BASE_DIR << "ickle.conf, using default settings" << endl
	 << "This is probably the first time you've run ickle.";
    SignalLog(LogEvent::WARN, ostr.str());
  }

  g_settings.defaultValueBool("away_autoposition", true);
  g_settings.defaultValueUnsignedInt("reconnect_retries", 2, 0, 10);
  g_settings.defaultValueBool("log_to_console", true);
  g_settings.defaultValueBool("log_to_file", false);
  g_settings.defaultValueUnsignedInt("geometry_width", 130, 30, 1000);
  g_settings.defaultValueUnsignedInt("geometry_height", 300, 30, 2000);
  g_settings.defaultValueUnsignedInt("geometry_x", 50);
  g_settings.defaultValueUnsignedInt("geometry_y", 50);
  g_settings.defaultValueUnsignedInt("autoconnect",STATUS_OFFLINE,STATUS_ONLINE,STATUS_OFFLINE);
  g_settings.defaultValueString("network_login_host", "login.icq.com");
  g_settings.defaultValueUnsignedShort("network_login_port", 5190, 1, 65535);
  g_settings.defaultValueBool("network_override_port", false);
  g_settings.defaultValueBool("message_autopopup", false);
  g_settings.defaultValueBool("message_autoraise", true);
  g_settings.defaultValueBool("message_autoclose", false);
  g_settings.defaultValueUnsignedInt("history_shownr", 10, 1, 255);
  g_settings.defaultValueString("message_header_font", "-*-*-bold-*-*-*-*-*-*-*-*-*-*-*");
  g_settings.defaultValueString("message_text_font", "");

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

  // --

  int width, height, x, y;
  width = g_settings.getValueUnsignedInt("geometry_width");
  height = g_settings.getValueUnsignedInt("geometry_height");
  x = g_settings.getValueUnsignedInt("geometry_x");
  y = g_settings.getValueUnsignedInt("geometry_y");
  gui.set_default_size( width, height );
  gui.set_uposition( x, y );

  g_icons.setIcons( g_settings.getValueString("icons_dir") );
  
  m_retries = g_settings.getValueUnsignedChar("reconnect_retries");
}

void IckleClient::saveSettings() {
  int width, height, x, y;
  gui.get_window().get_root_origin(x, y);
  gui.get_window().get_size(width, height);
  g_settings.setValue( "geometry_x", x );
  g_settings.setValue( "geometry_y", y );
  g_settings.setValue( "geometry_width", width );
  g_settings.setValue( "geometry_height", height );

  if ( mkdir( BASE_DIR.c_str(), 0700 ) == -1 && errno != EEXIST ) {
    ostringstream ostr;
    ostr << "mkdir " << BASE_DIR << " failed: " << strerror(errno);
    SignalLog(LogEvent::ERROR, ostr.str());
    return;
  }
  
  string ickle_conf = BASE_DIR + "ickle.conf";

  // set umask to secure value, so that if ickle.conf doesn't exist, and is created it will be safe.
  mode_t old_umask = umask(0077);

  if ( !g_settings.save(ickle_conf) ) {
    ostringstream ostr;
    ostr << "Couldn't save " << BASE_DIR << "ickle.conf";
    SignalLog(LogEvent::ERROR, ostr.str());
  }

  umask(old_umask);

  // ensure permissions on ickle.conf are secure
  if ( chmod( ickle_conf.c_str(), S_IRUSR | S_IWUSR ) == -1 ) {
    ostringstream ostr;
    ostr << "The permissions on " << ickle_conf << " couldn't be set to 0600. Your ICQ password is vulnerable!";
    SignalLog(LogEvent::ERROR, ostr.str());
  }

}

gint IckleClient::close_cb(GdkEventAny*) {
  /*
   * These both need to be done while the widget
   * still exists, in IckleClient::quit is too late
   */

  saveSettings();
  icqclient.setStatus(STATUS_OFFLINE);
  
  return false;
}

void IckleClient::quit() {
#ifdef GNOME_ICKLE
  applet.quit();
#else
  Gtk::Main::quit();
#endif
}

void IckleClient::connected_cb(ConnectedEvent *c) {
  /* the library needs to be polled regularly
   * to ensure timeouts are respected and the server is pinged every minute
   * I suggest a graularity of 5 seconds is sensible
   */
  poll_server_cnt = Gtk::Main::timeout.connect( slot( this, &IckleClient::poll_server_cb ), 5000 );
  m_retries = g_settings.getValueUnsignedChar("reconnect_retries");
}

void IckleClient::disconnected_cb(DisconnectedEvent *c) {
  if (c->getReason() == DisconnectedEvent::REQUESTED) {
    SignalLog(LogEvent::INFO, "Disconnected as requested");
  } else if (c->getReason() == DisconnectedEvent::FAILED_DUALLOGIN) {
    SignalLog(LogEvent::ERROR, "Dual login, disconnected");
  } else {
    ostringstream ostr;
    ostr << "Problem connecting: ";
    switch(c->getReason()) {
    case DisconnectedEvent::FAILED_LOWLEVEL:
      ostr << "Socket problems";
      break;
    case DisconnectedEvent::FAILED_BADUSERNAME:
      ostr << "Bad Username";
      break;
    case DisconnectedEvent::FAILED_TURBOING:
      ostr << "Turboing";
      break;
    case DisconnectedEvent::FAILED_BADPASSWORD:
      ostr << "Bad Password";
      break;
    case DisconnectedEvent::FAILED_MISMATCH_PASSWD:
      ostr << "Username and Password did not match";
      break;
    case DisconnectedEvent::FAILED_UNKNOWN:
      ostr << "Unknown";
      break;
    }
    SignalLog(LogEvent::ERROR, ostr.str() );
  }

  // disconnect PingServer callback
  poll_server_cnt.disconnect();
  
  if (m_retries > 0 && c->getReason() != DisconnectedEvent::REQUESTED) {
    --m_retries;
    Gtk::Main::idle.connect( bind( slot( this, &IckleClient::idle_reconnect_cb ), icqclient.getStatus() ) );
  }
}

gint IckleClient::idle_reconnect_cb(Status s) {
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

void IckleClient::logger_cb(LogEvent *c) {

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
  case LogEvent::INFO:
    if (!g_settings.getValueBool("log_info")) return;
    break;
  case LogEvent::ERROR:
    if (!g_settings.getValueBool("log_error")) return;
    break;
  case LogEvent::WARN:
    if (!g_settings.getValueBool("log_warn")) return;
    break;
  case LogEvent::PACKET:
    if (!g_settings.getValueBool("log_packet")) return;
    break;
  case LogEvent::DIRECTPACKET:
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
    case LogEvent::INFO:
      cout << "[34m (i)  ";
      break;
    case LogEvent::ERROR:
      cout << "[31m :-<  ";
      break;
    case LogEvent::WARN:
      cout << "[36m :-%  ";
      break;
    case LogEvent::PACKET:
      cout << "[32m [+]  ";
      break;
    case LogEvent::DIRECTPACKET:
      cout << "[32m {~}  ";
      break;
    }
    cout << c->getMessage() << endl;
    cout << "[39m";
  }

}

void IckleClient::socket_select_cb(int source, GdkInputCondition cond) {
  /*
  ostringstream ostr;
  ostr << "IckleClient::socket_cb " << source << " cond: " << cond;
  SignalLog(LogEvent::INFO, ostr.str());
  */

  icqclient.socket_cb(source, (SocketEvent::Mode)cond);
}

int IckleClient::poll_server_cb() {
  icqclient.Poll();
  return true;
}

void IckleClient::user_popup_cb(unsigned int uin) {
  Contact *c = icqclient.getContact(uin);
  if (c != NULL) {
    gui.popup_messagebox(c, m_histmap[c->getUIN()]);
  }
}

void IckleClient::userinfo_cb(unsigned int uin) {
  Contact *c = icqclient.getContact(uin);
  if (c != NULL) {
    gui.userinfo_popup(c);
  }
}

void IckleClient::send_event_cb(MessageEvent *ev) {
  icqclient.SendEvent(ev);
}

void IckleClient::messageack_cb(MessageEvent *ev) {
  if (ev->isFinished() && ev->isDelivered() && ev->getType() != MessageEvent::AwayMessage)
    m_histmap[ev->getContact()->getUIN()]->log(ev, false);
}

void IckleClient::add_user_cb(unsigned int uin) {
  Contact *c = icqclient.getContact(uin);
  if (c == NULL) {
    Contact nc(uin);
    icqclient.addContact(nc);
  }
}

void IckleClient::add_mobile_user_cb(string alias, string mobile_no) {
  Contact nc( alias, mobile_no );
  icqclient.addContact( nc );
}

void IckleClient::socket_cb(SocketEvent *ev) {
  
  if (dynamic_cast<AddSocketHandleEvent*>(ev) != NULL) {
    AddSocketHandleEvent *cev = dynamic_cast<AddSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    /*
    ostringstream ostr;
    ostr << "Connecting socket " << fd;
    if (cev->isRead()) ostr << " for read";
    if (cev->isWrite()) ostr << " for write";
    if (cev->isException()) ostr << " for exception";
    SignalLog(LogEvent::INFO, ostr.str());
    */

    if (m_sockets.count(fd) > 0) {
      // uh oh..
      SignalLog(LogEvent::ERROR, "Problem: file descriptor already connected");
      m_sockets[fd].disconnect();
      m_sockets.erase(fd);
    }

    m_sockets[fd] = Gtk::Main::input.connect(slot(this, &IckleClient::socket_select_cb), fd,
					     (GdkInputCondition)
					     ((cev->isRead() ? GDK_INPUT_READ : 0) |
					      (cev->isWrite() ? GDK_INPUT_WRITE : 0) |
					      (cev->isException() ? GDK_INPUT_EXCEPTION : 0)));

  } else if (dynamic_cast<RemoveSocketHandleEvent*>(ev) != NULL) {
    RemoveSocketHandleEvent *cev = dynamic_cast<RemoveSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    /*
    ostringstream ostr;
    ostr << "Disconnecting socket " << fd;
    SignalLog(LogEvent::INFO, ostr.str());
    */

    if (m_sockets.count(fd) == 0) {
      SignalLog(LogEvent::ERROR, "Problem: file descriptor not connected");
    } else {
      m_sockets[fd].disconnect();
      m_sockets.erase(fd);
    }
  }
}

bool IckleClient::message_cb(MessageEvent *ev) {
  if (ev->getType() == MessageEvent::Normal) {
    event_system("event_message", ev);
  } else if (ev->getType() == MessageEvent::URL) {
    event_system("event_url", ev);
  } else if (ev->getType() == MessageEvent::SMS) {
    event_system("event_sms", ev);
  }
  m_histmap[ev->getContact()->getUIN()]->log(ev, true);

  return false;
}

void IckleClient::event_system(const string& s, MessageEvent *ev) {
  if (g_settings.getValueString(s) != "") {
    Contact *co = ev->getContact();

    int ci;
    unsigned char c;
    char timebuf[100];
    time_t ev_time;
    istringstream istr (g_settings.getValueString(s));
    ostringstream ostr;

    while(istr.good()){
      ci = istr.get();
      if (ci == EOF)
        break;
      c = (unsigned char)ci;
      if (c == '%'){
        ci = istr.get();
	if (ci == EOF)
	  break;
	c = (unsigned char)ci;
        switch(c) {
	case 'i':
	  ostr << IPtoString( co->getExtIP() );
	  break;
	case 'p':
	  ostr << co->getExtPort();
	  break;
	case 'e':
	  ostr << co->getEmail();
	  break;
	case 'n':
	  ostr << co->getFirstName()
	       << " " << co->getLastName();
	  break;
	case 'f':
	  ostr << co->getFirstName();
	  break;
	case 'l':
	  ostr << co->getLastName();
	  break;
	case 'a':
	  ostr << co->getAlias();
	  break;
	case 'u':
	  ostr << co->getStringUIN();
	  break;
	  // case 'w': would be the web address
	  // case 'h': would be home phone number
	case 'c':
	  ostr << co->getMobileNo();
	  break;
	case 's':
	case 'S':
	  ostr << co->getStatusStr();
	  break;
	case 't':
	  ev_time = ev->getTime();
	  strftime(timebuf, 100, "%b %d %r", localtime(&ev_time));
	  ostr << timebuf;
	  break;
	case 'T':
	  ev_time = ev->getTime();
	  strftime(timebuf, 100, "%b %d %R %Z", localtime(&ev_time));
	  ostr << timebuf;
	  break;
	  // case 'o' would be last time they were online
	case 'm':
	  ostr << co->numberPendingMessages();
	  break;
	case '%':
	  ostr << "%";
	  break;
	default:
	  {
	    ostringstream ostr;
	    ostr << "Warning: no substitution for %" << c;
	    SignalLog(LogEvent::WARN, ostr.str());
	    break;
	  }
        }
      } else {
        ostr << c;
      }
    }
    ostr << " &";
    system(ostr.str().c_str());
  }
}

void IckleClient::contactlist_cb(ContactListEvent *ev) {
  Contact *c = ev->getContact();

  if (ev->getType() == ContactListEvent::UserAdded ||
      ev->getType() == ContactListEvent::UserInfoChange) {

    if (ev->getType() == ContactListEvent::UserAdded) {
      if (m_fmap.count(c->getUIN()) > 0) return;
      ostringstream ostr;

      ostr << CONTACT_DIR << c->getUIN() << ".user";

      int n = 0;
      struct stat fs;
      string filename;
      filename = ostr.str();
      while ( stat( filename.c_str(), &fs ) == 0 ) {
	ostringstream ostr;
	n++;
	ostr << CONTACT_DIR << c->getUIN() << "-" << n << ".user";
	filename = ostr.str();
      }
      // ensure uniqueness
      m_fmap[c->getUIN()] = filename;
      m_histmap[c->getUIN()] = new History( c->getUIN() );
    }

    if ( mkdir( BASE_DIR.c_str(), 0700 ) == -1 && errno != EEXIST ) {
      ostringstream ostr;
      ostr << "mkdir " << BASE_DIR << " failed: " << strerror(errno);
      SignalLog(LogEvent::ERROR, ostr.str());
      return;
    }
    if ( mkdir( CONTACT_DIR.c_str(), 0700 ) == -1 && errno != EEXIST ) {
      ostringstream ostr;
      ostr << "mkdir " << CONTACT_DIR << " failed: " << strerror(errno);
      SignalLog(LogEvent::ERROR, ostr.str());
      return;
    }
    
    Settings user;
    user.setValue( "alias", c->getAlias() );
    if (c->isICQContact()) user.setValue( "uin", c->getUIN() );
    user.setValue( "mobile_no", c->getMobileNo() );
    user.setValue( "firstname", c->getFirstName() );
    user.setValue( "lastname", c->getLastName() );
    user.setValue( "email", c->getEmail() );

    // Main Home Info
    MainHomeInfo& mhi = c->getMainHomeInfo();
    user.setValue( "city", mhi.city );
    user.setValue( "state", mhi.state );
    user.setValue( "phone", mhi.phone );
    user.setValue( "fax", mhi.fax );
    user.setValue( "street", mhi.street );
    user.setValue( "zip", mhi.zip );
    user.setValue( "country", mhi.country );
    user.setValue( "gmt", mhi.gmt );
    
    // Homepage Info
    HomepageInfo& hpi = c->getHomepageInfo();
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

    user.save(m_fmap[c->getUIN()]);

  } else if (ev->getType() == ContactListEvent::UserRemoved) {
    // delete
    unlink( m_fmap[c->getUIN()].c_str() );
  }
}

void IckleClient::settings_changed_cb() {
  saveSettings();
}

void IckleClient::fetch_cb(Contact* c) {
  icqclient.fetchDetailContactInfo(c);
}
