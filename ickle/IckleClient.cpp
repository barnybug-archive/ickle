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

#include "sstream_fix.h"

#include "main.h"
#include "Icons.h"
#include "Dir.h"

#include "Client.h"

#include <time.h>

using std::ostringstream;
using std::istringstream;

IckleClient::IckleClient(int argc, char* argv[])
  : gui(),
    status(STATUS_OFFLINE)
{
  // process command line parameters
  processCommandLine(argc,argv);

#ifdef GNOME_ICKLE
  // initialize GNOME applet
  applet.init(argc, argv, gui);
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
  icqclient.away_message.connect(slot(this,&IckleClient::away_message_cb));

  // set up GUI callbacks
  gui.settings_changed.connect(slot(this,&IckleClient::settings_changed_cb));
  gui.fetch.connect( slot( this, &IckleClient::fetch_cb ) );
  gui.getContactListView()->user_popup.connect( slot( this,&IckleClient::user_popup_cb ) );
  gui.getContactListView()->user_info.connect( slot( this, &IckleClient::user_info_cb ) );

  gui.send_event.connect(slot(this,&IckleClient::send_event_cb));
  gui.add_user.connect(slot(this,&IckleClient::add_user_cb));
  gui.add_mobile_user.connect(slot(this,&IckleClient::add_mobile_user_cb));

  gui.setDisplayTimes(true);

  loadSettings();

  // setup contact list
  loadContactList();

  gui.show_all();

  Status st = Status(g_settings.getValueUnsignedInt("autoconnect",STATUS_OFFLINE,STATUS_ONLINE,STATUS_OFFLINE));
  if (st != STATUS_OFFLINE) icqclient.setStatus(st);
}

IckleClient::~IckleClient() {
  g_icons.FreeIcons();
}

void IckleClient::loadContactList() {
  Dir dir;
  dir.list( CONTACT_DIR + "*.user" );

  Dir::iterator dirit = dir.begin();
  while( dirit != dir.end() ) {
    
    Settings cs;
    if (cs.load(*dirit)) {
      unsigned int uin = cs.getValueUnsignedInt("uin");
      if (uin != 0) {
	Contact c(uin);
	
	string alias = cs.getValueString("alias");
	if (!alias.empty()) c.setAlias(alias);
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
	hpi.age = cs.getValueUnsignedChar("age", 0, 0, 150);
	hpi.sex = cs.getValueUnsignedChar("sex", 0, 0, 2);
	hpi.homepage = cs.getValueString("homepage");
	hpi.birth_year = cs.getValueUnsignedShort("birth_year");
	hpi.birth_month = cs.getValueUnsignedChar("birth_month", 0, 0, 12);
	hpi.birth_day = cs.getValueUnsignedChar("birth_day", 0, 0, 31);
	hpi.lang1 = cs.getValueUnsignedChar("lang1");
	hpi.lang2 = cs.getValueUnsignedChar("lang2");
	hpi.lang3 = cs.getValueUnsignedChar("lang3");

	// About Info
	c.setAboutInfo( cs.getValueString("about") );
	
	m_fmap[c.getUIN()] = *dirit;
	m_histmap[c.getUIN()] = History( &c );
	icqclient.addContact(c);
      } else {
	Contact c( cs.getValueString("alias"), cs.getValueString("mobile_no") );
	m_fmap[c.getUIN()] = *dirit;
	m_histmap[c.getUIN()] = History( &c );
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


void IckleClient::loadSettings() {
  // load in settings
  if (!g_settings.load(BASE_DIR + "ickle.conf")) {
    cout << "Couldn't open " << BASE_DIR << "ickle.conf, using default settings" << endl
	 << "This is probably the first time you've run ickle." << endl;
  } else {
    icqclient.setUIN(g_settings.getValueUnsignedInt("uin"));
    icqclient.setPassword(g_settings.getValueString("password"));
    icqclient.setTranslationMap( g_settings.getValueString("translation_map") );
  }

  int width, height, x, y;
  width = g_settings.getValueUnsignedInt("geometry_width", 130, 30, 1000);
  height = g_settings.getValueUnsignedInt("geometry_height", 300, 30, 2000);
  x = g_settings.getValueUnsignedInt("geometry_x", 50);
  y = g_settings.getValueUnsignedInt("geometry_y", 50);
  gui.set_default_size( width, height );
  gui.set_uposition( x, y );

  g_icons.setIcons( g_settings.getValueString("icons_dir") );
  
  g_settings.defaultValue("away_autoposition", true);

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
    cout << "mkdir " << BASE_DIR << " failed: " << strerror(errno) << endl;
    return;
  }
  
  string ickle_conf = BASE_DIR + "ickle.conf";

  // set umask to secure value, so that if ickle.conf doesn't exist, and is created it will be safe.
  mode_t old_umask = umask(0077);

  if ( !g_settings.save(ickle_conf) ) {
    cout << "Couldn't save " << BASE_DIR << "ickle.conf" << endl;
  }

  umask(old_umask);

  // ensure permissions on ickle.conf are secure
  if ( chmod( ickle_conf.c_str(), S_IRUSR | S_IWUSR ) == -1 ) {
    cout << "The permissions on " << ickle_conf << " couldn't be set to 0600. Your ICQ password is vulnerable!" << endl;
  }

}

gint IckleClient::close_cb(GdkEventAny*) {
  /*
   * These both need to be done while the widget
   * still exists, in IckleClient::quit is too late
   */

  saveSettings();
  icqclient.Disconnect();
  
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
}

void IckleClient::disconnected_cb(DisconnectedEvent *c) {
  if (c->getReason() == DisconnectedEvent::REQUESTED) {
    cout << "ickle: Disconnected as requested" << endl;
  } else if (c->getReason() == DisconnectedEvent::FAILED_DUALLOGIN) {
    cout << "ickle: Dual login, disconnected" << endl;
  } else {
    cout << "ickle: Problem connecting: ";
    switch(c->getReason()) {
    case DisconnectedEvent::FAILED_LOWLEVEL:
      cout << "Socket problems";
      break;
    case DisconnectedEvent::FAILED_BADUSERNAME:
      cout << "Bad Username";
      break;
    case DisconnectedEvent::FAILED_TURBOING:
      cout << "Turboing";
      break;
    case DisconnectedEvent::FAILED_BADPASSWORD:
      cout << "Bad Password";
      break;
    case DisconnectedEvent::FAILED_MISMATCH_PASSWD:
      cout << "Username and Password did not match";
      break;
    case DisconnectedEvent::FAILED_UNKNOWN:
      cout << "Unknown";
      break;
    }
    cout << endl;
  }

  // disconnect PingServer callback
  poll_server_cnt.disconnect();

}

void IckleClient::logger_cb(LogEvent *c) {

  switch(c->getType()) {
  case LogEvent::INFO:
    cout << "[34m";
    break;
  case LogEvent::WARN:
    cout << "[31m";
    break;
  case LogEvent::PACKET:
    cout << "[32m";
    break;
  }

  cout << c->getMessage() << endl;
  cout << "[39m";
}

void IckleClient::socket_select_cb(int source, GdkInputCondition cond) {
  icqclient.socket_cb(source, (SocketEvent::Mode)cond);
}

int IckleClient::poll_server_cb() {
  icqclient.Poll();
  return true;
}

void IckleClient::user_popup_cb(unsigned int uin) {
  Contact *c = icqclient.getContact(uin);
  if (c != NULL) {
    gui.user_popup(c);
  }
}

void IckleClient::user_info_cb(unsigned int uin) {
  Contact *c = icqclient.getContact(uin);
  if (c != NULL) {
    gui.user_info_edit(c);
  }
}

void IckleClient::send_event_cb(MessageEvent *ev) {
  icqclient.SendEvent(ev);
}

void IckleClient::messageack_cb(MessageEvent *ev) {
  if (ev->isFinished() && ev->isDelivered())
    m_histmap[ev->getContact()->getUIN()].log(ev, false);
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

    cout << "connecting socket " << fd << endl;

    if (m_sockets.count(fd) > 0) {
      // uh oh..
      cout << "problem: file descriptor already connected" << endl;
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

    cout << "disconnecting socket " << fd << endl;

    m_sockets[fd].disconnect();
    m_sockets.erase(fd);
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
  m_histmap[ev->getContact()->getUIN()].log(ev, true);

  return false;
}

void IckleClient::event_system(const string& s, MessageEvent *ev) {
  if (g_settings.getValueString(s) != "") {
    Contact *co = ev->getContact();

    char c;
    char timebuf[100];
    time_t ev_time;
    istringstream istr (g_settings.getValueString(s));
    ostringstream ostr;

    while(istr.good()){
      c = istr.get();
      if(c == -1)
        break;
      if(c=='%'){
        c = istr.get();
        switch(c){
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
	  cout << "Warning: no substitution for %" << c << endl;
	  break;
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
      m_histmap[c->getUIN()] = History( c );
    }

    if ( mkdir( BASE_DIR.c_str(), 0700 ) == -1 && errno != EEXIST ) {
      cout << "mkdir " << BASE_DIR << " failed: " << strerror(errno) << endl;
      return;
    }
    if ( mkdir( CONTACT_DIR.c_str(), 0700 ) == -1 && errno != EEXIST ) {
      cout << "mkdir " << CONTACT_DIR << " failed: " << strerror(errno) << endl;
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

void IckleClient::away_message_cb(AwayMsgEvent *ev) {
  cout << "Away Message: " << ev->getMessage() << endl;
}

void IckleClient::settings_changed_cb() {
  saveSettings();
}

void IckleClient::fetch_cb(Contact* c) {
  icqclient.fetchSimpleContactInfo(c);
}
