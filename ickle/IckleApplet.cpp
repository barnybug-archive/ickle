/* $Id: IckleApplet.cpp,v 1.3 2001-11-22 20:54:43 nordman Exp $
 * IckleApplet.cpp
 *
 * GNOME applet for ickle.
 *
 * Copyright (C) 2001 Nils Nordman <nino@nforced.com>
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

#include <config.h>
#include <gnome.h>
#include <sstream>

#include <constants.h>
#include "main.h"
#include "IckleApplet.h"
#include "Icons.h"

using std::ostringstream;

#define PANEL_PADDING 10

void IckleApplet::applet_click_converter(GtkWidget *sender, GdkEventButton *ev, gpointer data)
{
  if (ev || ev->button == 1 || ev->type == GDK_BUTTON_PRESS)
    ((IckleApplet *)data)->applet_click_cb();
}


void IckleApplet::applet_sizechange_converter(GtkWidget *w, int size, gpointer data)
{
    ((IckleApplet *)data)->applet_sizechange_cb(size);
}
 

void IckleApplet::applet_click_cb()
{
  if( m_gui->is_visible() ) {
    
    m_gui->hide();

    if( m_nrmsg ) {
      // assume user has read the messages now (FIXME: a lot better handling for this please)
      ostringstream ostr;
      Gtk::ImageLoader *il = Icons::IconForStatus( icqclient.getStatus(), icqclient.getInvisible() );
      m_pm.set( il->pix(), il->bit() );
      m_nrmsg = 0;
      ostr << m_nrmsg;
      m_nr.set_text( ostr.str() );
    }
  }
  else {
    m_gui->show();
  }
}


void IckleApplet::applet_sizechange_cb(int size)
{
  size -= PANEL_PADDING;
  m_frame.set_usize(-1,size);
}


void IckleApplet::applet_status_menu_cb(AppletWidget *applet, gpointer data)
{
  Status st = (Status)(int)data;
  if( st == STATUS_OFFLINE ) {
    if( icqclient.getStatus() != STATUS_OFFLINE )
      icqclient.Disconnect();
  }
  else {
    icqclient.setStatus( st );
    if( icqclient.getStatus() == STATUS_OFFLINE )
      icqclient.Connect();
  }
}


gint IckleApplet::applet_delete_cb(GtkWidget *widget, GdkEvent  *event, gpointer data)
{
  ((IckleApplet *)data)->m_applet = NULL;
  return FALSE;
}


bool IckleApplet::icq_messaged_cb(MessageEvent* ev)
{
  ostringstream ostr;
  
  if( !m_nrmsg++ ) {
    Gtk::ImageLoader *il = Icons::IconForEvent( ev->getType() );
    m_pm.set( il->pix(), il->bit() );
  }
  ostr << m_nrmsg;
  m_nr.set_text( ostr.str() );
  return false;
}


void IckleApplet::icq_statuschanged_cb(MyStatusChangeEvent *ev)
{
  Gtk::ImageLoader *il;

  il = Icons::IconForStatus( ev->getStatus(), icqclient.getInvisible() );
  m_pm.set( il->pix(), il->bit() );
  update_tooltip( ev->getStatus() );
}


void IckleApplet::update_tooltip(Status st)
{
  ostringstream ostr;
  ostr << icqclient.getUIN() << " " << Status_text[st];
  applet_widget_set_tooltip( APPLET_WIDGET(m_applet), ostr.str().c_str() );
}


IckleApplet::IckleApplet()
  : m_hbox(false, 0)
{
  m_nr.set_text( "0" );
  m_nrmsg = 0;
  m_applet = NULL;
}


void IckleApplet::init(int argc, char* argv[], IckleGUI &g)
{
  Gtk::ImageLoader *il;

  m_gui = &g;

  // setup callbacks
  icqclient.statuschanged.connect( slot(this, &IckleApplet::icq_statuschanged_cb) );
  icqclient.messaged.connect(slot(this,&IckleApplet::icq_messaged_cb));

  // create applet

  // FIXME: applet_widget_init will not recognize ickle's commandline options
  applet_widget_init("ickle_applet", NULL, argc, argv, NULL,0,NULL);
  m_applet = applet_widget_new("ickle_applet");
  if(!m_applet)
    g_error("Can't create applet!\n");

  // connect applet signals
  gtk_widget_set_events(m_applet, gtk_widget_get_events(m_applet) | GDK_BUTTON_PRESS_MASK);
  gtk_widget_realize(m_applet);
  gtk_signal_connect(GTK_OBJECT(m_applet), "button_press_event",
		     GTK_SIGNAL_FUNC(applet_click_converter), this);
   gtk_signal_connect(GTK_OBJECT(m_applet), "delete_event",
  		     GTK_SIGNAL_FUNC(applet_delete_cb), this);

#ifdef HAVE_PANEL_PIXEL_SIZE
  gtk_signal_connect(GTK_OBJECT(m_applet),"change_pixel_size",
		     GTK_SIGNAL_FUNC(applet_sizechange_converter), this);
#endif

  // create context menus
  applet_widget_register_callback_dir( APPLET_WIDGET(m_applet), "status", _("Status"));
  applet_widget_register_callback( APPLET_WIDGET(m_applet), "status/online",
				   Status_text[STATUS_ONLINE], applet_status_menu_cb, (void *)STATUS_ONLINE);
  applet_widget_register_callback( APPLET_WIDGET(m_applet), "status/away",
				   Status_text[STATUS_AWAY], applet_status_menu_cb, (void *)STATUS_AWAY);
  applet_widget_register_callback( APPLET_WIDGET(m_applet), "status/na",
				   Status_text[STATUS_NA], applet_status_menu_cb, (void *)STATUS_NA);
  applet_widget_register_callback( APPLET_WIDGET(m_applet), "status/occupied",
				   Status_text[STATUS_OCCUPIED], applet_status_menu_cb, (void *)STATUS_OCCUPIED);
  applet_widget_register_callback( APPLET_WIDGET(m_applet), "status/dnd",
				   Status_text[STATUS_DND], applet_status_menu_cb, (void *)STATUS_DND);
  applet_widget_register_callback( APPLET_WIDGET(m_applet), "status/freeforchat",
				   Status_text[STATUS_FREEFORCHAT], applet_status_menu_cb, (void *)STATUS_FREEFORCHAT);
  applet_widget_register_callback( APPLET_WIDGET(m_applet), "status/offline",
				   Status_text[STATUS_OFFLINE], applet_status_menu_cb, (void *)STATUS_OFFLINE);

  // whip the layout together
  il = Icons::IconForStatus( STATUS_OFFLINE, false );
  m_pm.set( il->pix(), il->bit() );
  m_hbox.pack_start( m_pm, true, true, 3 );
  m_hbox.pack_end( m_nr, true, true, 3 );
  m_frame.add(m_hbox);

  applet_widget_add(APPLET_WIDGET(m_applet), GTK_WIDGET(m_frame.gtkobj()) );
  update_tooltip( STATUS_OFFLINE );

  gtk_widget_show_all(m_applet);
}

void IckleApplet::quit()
{
  if( m_applet != NULL )
    applet_widget_remove( APPLET_WIDGET(m_applet) );
}
