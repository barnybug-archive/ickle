/*
 * ickle
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


#include <iostream>

#include <gtkmm/main.h>

#include "ickle.h"
#include "ucompose.h"
#include "main.h"
#include "IckleClient.h"

#include <libicq2000/Client.h>

using std::string;
using std::cout;
using std::endl;

class ICQ2000::Client icqclient;

class Settings g_settings;
class Icons g_icons;

string BASE_DIR;
string CONTACT_DIR;
string DATA_DIR;
string TRANSLATIONS_DIR;
string ICONS_DIR;
string PID_FILENAME;

int main(int argc, char* argv[])
{
  try
  {
#ifdef ENABLE_NLS
    /* initialise gettext */
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);
#endif

    Gtk::Main gtkmain(argc,argv,true);
    g_icons.setDefaultIcons();
    
    IckleClient client(argc, argv);
    if (!client.check_pid_file()) return -1;
    client.init();    // finish initialising

    gtkmain.run();

    return 0;
  }
  catch( std::exception &e )
  {
    cout << String::ucompose(_("Exiting abnormally: %1"), e.what()) << endl;
  }
  catch( ... )
  {
    cout << Glib::ustring(_("Exiting abnormally")) << endl;
  }
  return 1;
}
