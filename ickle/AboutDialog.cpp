/* $Id: AboutDialog.cpp,v 1.3 2002-01-16 19:30:51 oizoken Exp $
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

#include "AboutDialog.h"

#include <gtk--/main.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/text.h>
#include <gtk--/scrollbar.h>
#include <gtk--/table.h>

#include <libicq2000/version.h>

#include "sstream_fix.h"

AboutDialog::AboutDialog()
  : Gtk::Dialog()
{
  Gtk::Button *button;
  
  set_title("About ickle");
  set_modal(true);

  Gtk::HBox *hbox = get_action_area();

  button = manage( new Gtk::Button("OK") );
  button->clicked.connect( destroy.slot() );
  hbox->pack_start( *button );
  
  Gtk::VBox *vbox = get_vbox();
  Gtk::Table *table = manage( new Gtk::Table(2,1) );

  Gtk::Text *text = manage( new Gtk::Text() );
  text->set_word_wrap(true);
  Gtk::Text_Helpers::Context hd, p;
  hd.set_foreground(Gdk_Color("red"));
  text->insert(hd, "About ickle\n");


  std::ostringstream ostr1;
  ostr1 << "Version: " << ICKLE_VERSION << std::endl;
  text->insert(p, ostr1.str() );

  std::ostringstream ostr2;
  ostr2 << "Compiled on: " << __DATE__ << std::endl;
  text->insert(p, ostr2.str() );

  std::ostringstream ostr3;
  ostr3 << "libicq2000 Version: " << libicq2000_version << std::endl;
  text->insert(p, ostr3.str() );

  text->insert(hd, "\nDevelopers\n");
  text->insert(p, "* Barnaby Gray <barnaby@beedesign.co.uk> ICQ: 12137587\n");
  text->insert(p, "* Nils Nordman <nino@nforced.com>\n");
  text->insert(p, "* Alex Antropoff <alex@tirastel.md>\n\n");
  text->insert(p, "Further contributions by many other developers are listed in the THANKS file.\n");
  text->insert(hd, "\nFurther information\n");
  text->insert(p, "If you'd like to comment on ickle, contribute to the project or file a bug report please see the README for more information.\n");

  // scrollbars
  Gtk::Scrollbar *scrollbar = manage( new Gtk::VScrollbar (*(text->get_vadjustment())) );
  table->attach(*text, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
  table->attach(*scrollbar, 1, 2, 0, 1, 0, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);

  vbox->pack_start( *table, true, true );

  set_border_width(10);
  set_usize(500, 300);
  show_all();
}

void AboutDialog::run() {
  Gtk::Main::run();
}
