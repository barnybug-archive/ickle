/* $Id: AboutDialog.cpp,v 1.14 2003-07-01 17:12:54 barnabygray Exp $
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

#include "ickle.h"
#include "ucompose.h"

using std::endl;

AboutDialog::AboutDialog(Gtk::Window& parent)
  : Gtk::Dialog( _("About ickle"), parent, true)
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

  iter = buffer->insert_with_tag(iter,
				 String::ucompose( _("Version: %1\n"), ICKLE_VERSION ),
				 p );

  
  iter = buffer->insert_with_tag(iter,
				 String::ucompose( _("Compiled on: %1\n"), __DATE__ ),
				 p );

  iter = buffer->insert_with_tag(iter,
				 String::ucompose( _("libicq2000 version: %1\n"), libicq2000_version ),
				 p );

  iter = buffer->insert_with_tag(iter, _("\nDevelopers\n"), hd );
  iter = buffer->insert_with_tag(iter, _("* Barnaby Gray <barnaby@beedesign.co.uk> ICQ: 12137587\n"), p );
  iter = buffer->insert_with_tag(iter, _("* Nils Nordman <nino@nforced.com> ICQ: 778602\n"), p );
  iter = buffer->insert_with_tag(iter, _("* Dominic Sacr\xC3\xA9 <bugcreator@gmx.de> ICQ: 102496033\n"), p );
  iter = buffer->insert_with_tag(iter, _("* Alex Antropoff <alex@tirastel.md>\n"), p );
  iter = buffer->insert_with_tag(iter, _("* Christian Borntr\xC3\xA4ger <linux@borntraeger.net> ICQ: 113774556\n\n"), p );
  iter = buffer->insert_with_tag(iter, _("Further contributions by many other developers are listed in the THANKS file.\n"), p );
  iter = buffer->insert_with_tag(iter, _("\nFurther information\n"), hd );
  iter = buffer->insert_with_tag(iter,
				 _("If you'd like to comment on ickle, contribute to the project or file a bug "
				   "report please see the README for more information.\n"), p );

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
