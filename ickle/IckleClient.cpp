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

#include <time.h>

IckleClient::IckleClient(int argc, char* argv[])
  : gui(),
    status(STATUS_OFFLINE)
{
  // setup default compiled in xpms
  Icons::DefaultIcons();

  // process command line parameters
  processCommandLine(argc,argv);

  // let us know when the gui is destroyed
  gui.close.connect(slot(this,&IckleClient::quit));
  
  // set up libICQ2000 Callbacks
  // -- callbacks into IckleClient
  icqclient.connected.connect(slot(this,&IckleClient::connected_cb));
  icqclient.disconnected.connect(slot(this,&IckleClient::disconnected_cb));
  icqclient.logger.connect(slot(this,&IckleClient::logger_cb));
  icqclient.contactlist.connect(slot(this,&IckleClient::contactlist_cb));
  icqclient.messaged.connect(slot(this,&IckleClient::message_cb));
  icqclient.socket.connect(slot(this,&IckleClient::socket_cb));

  // set up GUI callbacks
  gui.status_changed.connect(slot(this,&IckleClient::status_change_cb));
  gui.fetch.connect( slot( this, &IckleClient::fetch_cb ) );
  gui.getContactListView()->user_popup.connect( slot( this,&IckleClient::user_popup_cb ) );
  gui.getContactListView()->user_info.connect( slot( this, &IckleClient::user_info_cb ) );

  gui.send_event.connect(slot(this,&IckleClient::send_event_cb));
  gui.add_user.connect(slot(this,&IckleClient::add_user_cb));
  gui.add_mobile_user.connect(slot(this,&IckleClient::add_mobile_user_cb));
  gui.settings.connect(slot(this,&IckleClient::settings_cb));

  gui.setDisplayTimes(true);

  loadSettings();

  // setup contact list
  loadContactList();

}

IckleClient::~IckleClient() {
  Icons::FreeIcons();
}

void IckleClient::loadContactList() {
  glob_t gr;
  string pattern( CONTACT_DIR + "*.user" );

  if ( glob( pattern.c_str(), 0, NULL, &gr ) == 0
       && gr.gl_pathc > 0 ) {
    char **nextp;
    nextp = gr.gl_pathv;

    while (*nextp != NULL) {
      Settings cs;
      if (cs.load(string(*nextp))) {
	unsigned int uin = cs.getValueUnsignedInt("uin");
	if (uin != 0) {
	  Contact c(uin);

	  string alias = cs.getValueString("alias");
	  if (!alias.empty()) c.setAlias(alias);
	  c.setMobileNo(cs.getValueString("mobile_no"));
	  c.setFirstName(cs.getValueString("firstname"));
	  c.setLastName(cs.getValueString("lastname"));
	  c.setEmail(cs.getValueString("email"));

	  m_fmap[c.getUIN()] = string(*nextp);
	  m_histmap[c.getUIN()] = history_filename( string(*nextp) );
	  icqclient.addContact(c);
	} else {
	  Contact c( cs.getValueString("alias"), cs.getValueString("mobile_no") );
	  m_fmap[c.getUIN()] = string(*nextp);
	  m_histmap[c.getUIN()] = history_filename( string(*nextp) );
	  icqclient.addContact(c);
	}
      }
      ++nextp;
    }
    
  }

  globfree( &gr );
}

History IckleClient::history_filename(const string& s) {
  ostringstream ostr;
  if ( s.find( ".user", s.size() - 5 ) != -1) {
    ostr << s.substr( 0, s.size() - 5 );
  } else {
    ostr << s;
  }
  ostr << ".history";
  return History( ostr.str() );
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
}


void IckleClient::loadSettings() {
  // load in settings
  if (!settings.load(BASE_DIR + "ickle.conf")) {
    cout << "Couldn't open " << BASE_DIR << "ickle.conf, using default settings" << endl;
  } else {
    icqclient.setUIN(settings.getValueUnsignedInt("uin"));
    icqclient.setPassword(settings.getValueString("password"));
  }

  int width, height, x, y;
  width = settings.getValueUnsignedInt("geometry_width", 130, 30, 1000);
  height = settings.getValueUnsignedInt("geometry_height", 300, 30, 2000);
  x = settings.getValueUnsignedInt("geometry_x", 50);
  y = settings.getValueUnsignedInt("geometry_y", 50);
  gui.set_default_size( width, height );
  gui.set_uposition( x, y );
  
}

void IckleClient::saveSettings() {
  int width, height, x, y;
  gui.get_window().get_origin(x, y);
  gui.get_window().get_size(width, height);
  settings.setValue( "geometry_x", x );
  settings.setValue( "geometry_y", y );
  settings.setValue( "geometry_width", width );
  settings.setValue( "geometry_height", height );

  if ( mkdir( BASE_DIR.c_str(), 0700 ) == -1 && errno != EEXIST ) {
    cout << "mkdir " << BASE_DIR << " failed: " << strerror(errno) << endl;
    return;
  }
  
  if ( !settings.save(BASE_DIR + "ickle.conf") ) {
    cout << "Couldn't save " << BASE_DIR << "ickle.conf" << endl;
  }
  
}

void IckleClient::quit() {
  saveSettings();

  icqclient.Disconnect();
  Gtk::Main::quit();   
}

void IckleClient::connected_cb(ConnectedEvent *c) {
  // setup PingServer timed callback
  ping_server_cnt = Gtk::Main::timeout.connect( slot( this, &IckleClient::ping_server_cb ), 60000 );

  gui.setStatus(icqclient.getStatus());
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
  ping_server_cnt.disconnect();

  // set status to offline
  gui.setStatus(STATUS_OFFLINE);
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

void IckleClient::status_change_cb(Status st) {
  if(st == STATUS_OFFLINE) {
    icqclient.Disconnect();
  } else {
    icqclient.setStatus(st);
    if (icqclient.isConnected()) {
      gui.setStatus(st);
    } else {
      if (icqclient.getUIN() == 0 || icqclient.getPassword() == "") {
	gui.setStatus(STATUS_OFFLINE);
	gui.invalid_login_prompt();
	return;
      }

      icqclient.Connect();
    }
  }
}

void IckleClient::socket_select_cb(int source, GdkInputCondition cond) {
  icqclient.socket_cb(source, (SocketEvent::Mode)cond);
}

int IckleClient::ping_server_cb() {
  icqclient.PingServer();
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

void IckleClient::settings_cb() {
  SettingsDialog dialog( settings );
  if (dialog.run()) {
    bool reconnect = false;
    if (dialog.getUIN() != icqclient.getUIN() ||
	dialog.getPassword() != icqclient.getPassword()) reconnect = icqclient.isConnected();

    if (reconnect) icqclient.Disconnect();
    
    dialog.updateSettings(settings);
    saveSettings();

    if (dialog.getUIN() != icqclient.getUIN()) icqclient.setUIN(dialog.getUIN());
    if (dialog.getPassword() != icqclient.getPassword()) icqclient.setPassword(dialog.getPassword());
  
    if (reconnect) status_change_cb(STATUS_ONLINE);
  }

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
  if (settings.getValueString(s) != "") {
    Contact *co = ev->getContact();

    char c;
    char timebuf[100];
    time_t ev_time;
    istringstream istr (settings.getValueString(s), istringstream::in);
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
           // Should implement a getStatusStr() for these
           // case 's':
           // case 'S':
           //  ostr << co->getStatus();
           //  break;
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

    if (ev->getType() == ContactListEvent::UserAdded && m_fmap.count(c->getUIN()) == 0) {
      ostringstream filename;

      filename << CONTACT_DIR << c->getUIN() << ".user";

      int n = 0;
      struct stat fs;
      while ( stat( filename.str().c_str(), &fs ) == 0 ) {
	n++;
	filename.seekp(0);
	filename << CONTACT_DIR << c->getUIN() << "-" << n << ".user";
	
      }
      // ensure uniqueness
      m_fmap[c->getUIN()] = filename.str();
      m_histmap[c->getUIN()] = history_filename( filename.str() );
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
    user.save(m_fmap[c->getUIN()]);

  } else if (ev->getType() == ContactListEvent::UserRemoved) {
    // delete
    unlink( m_fmap[c->getUIN()].c_str() );
  }
}

void IckleClient::fetch_cb(Contact* c) {
  icqclient.fetchSimpleContactInfo(c);
}
