#include <iostream>

#include <gtk--/main.h>

#include "main.h"
#include "IckleClient.h"

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

#include <gtk--/main.h>

#include "main.h"
#include "IckleClient.h"

#ifdef GNOME_ICKLE
# include <applet-widget.h>
#endif

class ICQ2000::Client icqclient;

class Settings g_settings;
class Icons g_icons;

string BASE_DIR;
string CONTACT_DIR;
string DATA_DIR;
string TRANSLATIONS_DIR;
string ICONS_DIR;

int main(int argc, char* argv[]) {
  try {
    Gtk::Main gtkmain(argc,argv,true);
    IckleClient client(argc,argv);
#ifdef GNOME_ICKLE
    applet_widget_gtk_main();
#else
    gtkmain.run();
#endif 
    return 0;
  }
  catch( exception &e ) {
    cout << "Exiting abnormally: " << e.what() << endl;
  }
  catch( ... ) {
    cout << "Exiting abnormally" << endl;
  }
  return 1;
}
