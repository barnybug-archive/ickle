/* $Id: AboutDialog.cpp,v 1.10 2003-01-02 16:39:40 barnabygray Exp $
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

#include <gtkmm/box.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>

#include <libicq2000/version.h>

#include "sstream_fix.h"

using std::ostringstream;
using std::endl;

AboutDialog::AboutDialog(Gtk::Window& parent)
  : Gtk::Dialog("About ickle", parent, true)
{
  set_position( Gtk::WIN_POS_CENTER );
  add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing (10);

  Gtk::TextView *textview = manage( new Gtk::TextView() );
  textview->set_editable(false);
  textview->set_cursor_visible(false);
  textview->set_wrap_mode(Gtk::WRAP_WORD);

  Glib::RefPtr<Gtk::TextBuffer> buffer = textview->get_buffer();
  Glib::RefPtr<Gtk::TextBuffer::Tag> hd, p;

  hd = buffer->create_tag("heading");
  p = buffer->create_tag("normal");
  
  hd->property_foreground().set_value( "red" );

  Gtk::TextBuffer::iterator iter = buffer->end();
  iter = buffer->insert_with_tag(iter, "About ickle\n", hd);

  ostringstream ostr1;
  ostr1 << "Version: " << ICKLE_VERSION << endl;
  iter = buffer->insert_with_tag(iter, ostr1.str(), p );

  ostringstream ostr2;
  ostr2 << "Compiled on: " << __DATE__ << endl;
  iter = buffer->insert_with_tag(iter, ostr2.str(), p );

  ostringstream ostr3;
  ostr3 << "libicq2000 Version: " << libicq2000_version << endl;
  iter = buffer->insert_with_tag(iter, ostr3.str(), p );

  iter = buffer->insert_with_tag(iter, "\nDevelopers\n", hd );
  iter = buffer->insert_with_tag(iter, "* Barnaby Gray <barnaby@beedesign.co.uk> ICQ: 12137587\n", p );
  iter = buffer->insert_with_tag(iter, "* Nils Nordman <nino@nforced.com> ICQ: 778602\n", p );
  iter = buffer->insert_with_tag(iter, "* Dominic Sacr\xC3\xA9 <bugcreator@gmx.de> ICQ: 102496033\n", p );
  iter = buffer->insert_with_tag(iter, "* Alex Antropoff <alex@tirastel.md>\n\n", p );
  iter = buffer->insert_with_tag(iter, "Further contributions by many other developers are listed in the THANKS file.\n", p );
  iter = buffer->insert_with_tag(iter, "\nFurther information\n", hd );
  iter = buffer->insert_with_tag(iter,
				 "If you'd like to comment on ickle, contribute to the project or file a bug "
				 "report please see the README for more information.\n", p );

  // scrollbars
  Gtk::ScrolledWindow *scr_win = manage( new Gtk::ScrolledWindow() );
  scr_win->set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS );
  scr_win->add(*textview);
  scr_win->set_shadow_type(Gtk::SHADOW_IN);
  vbox->pack_start( *scr_win, true, true );

  set_border_width(10);
  set_default_size(500, 300);
  show_all();
}
