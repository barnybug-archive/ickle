/* $Id: IckleApplet.cpp,v 1.4 2001-11-23 19:36:37 nordman Exp $
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
  if (ev && ev->button == 1 && ev->type == GDK_BUTTON_PRESS)
    ((IckleApplet *)data)->applet_click_cb();
}


void IckleApplet::applet_sizechange_converter(GtkWidget *w, int size, gpointer data)
{
    ((IckleApplet *)data)->applet_sizechange_cb(size);
}
 

void IckleApplet::applet_click_cb()
{
  if( m_gui->is_visible() )
    m_gui->hide();
  else
    m_gui->show();
}


void IckleApplet::applet_sizechange_cb(int size)
{
  size -= PANEL_PADDING;
  m_frame.set_usize(-1,size);
}


void IckleApplet::applet_status_menu_cb(AppletWidget *applet, gpointer data)
{
  Status st = (Status)(int)data;
  icqclient.setStatus( st );
}


gint IckleApplet::applet_delete_cb(GtkWidget *widget, GdkEvent  *event, gpointer data)
{
  ((IckleApplet *)data)->m_applet = NULL;
  return FALSE;
}


bool IckleApplet::icq_messaged_cb(MessageEvent* ev)
{
  Contact *c = ev->getContact();
  ostringstream ostr;
  
  if( !m_nr_msgs++ ) {
    Gtk::ImageLoader *il = Icons::IconForEvent( ev->getType() );
    m_pm.set( il->pix(), il->bit() );
  }

  // record in msg list
  list<msg_entry>::iterator itr = m_pending.begin();
  for( ; itr->uin != c->getUIN() && itr != m_pending.end(); ++itr );
  if( itr == m_pending.end() )
    m_pending.push_back( msg_entry( c->getUIN(), c->getAlias() ) );
  else
    ++itr->nr_msgs;
  
  update_applet_number();
  update_applet_tooltip();
  return false;
}


void IckleApplet::icq_statuschanged_cb(MyStatusChangeEvent *ev)
{
  Gtk::ImageLoader *il;

  if( !m_nr_msgs ) { // we don't even display the status for (m_nr_msgss > 0)
    il = Icons::IconForStatus( ev->getStatus(), icqclient.getInvisible() );
    m_pm.set( il->pix(), il->bit() );
  }
  update_applet_tooltip();
}


void IckleApplet::icq_contactlist_cb(ContactListEvent *ev)
{
  Contact *c = ev->getContact();
  
  if( ev->getType() == ContactListEvent::StatusChange ) {
    StatusChangeEvent *sc = dynamic_cast<StatusChangeEvent *>(ev);
    if( !sc )
      g_error( "Can't cast to StatusChangeEvent *" );
    if( sc->getOldStatus() == STATUS_OFFLINE )
      ++m_nr_online_users;
    else if (sc->getStatus() == STATUS_OFFLINE )
      --m_nr_online_users;
  }
  else if( ev->getType() == ContactListEvent::UserAdded ) {
    ++m_nr_users;
  }
  else if( ev->getType() == ContactListEvent::UserRemoved ) {
    --m_nr_users;
  }
  else if( ev->getType() == ContactListEvent::MessageQueueChanged ) {
    
    // find entry for this UIN in msg list
    list<msg_entry>::iterator itr = m_pending.begin();
    for( ; itr->uin != c->getUIN() && itr != m_pending.end(); ++itr );

    if( itr == m_pending.end() ) // no UIN?!
      g_warning( "empty message queue changed for uin %i", c->getUIN()  );
    else {
      m_nr_msgs -= (itr->nr_msgs - c->numberPendingMessages());
      if( !c->numberPendingMessages() ) // no more pending messages from this contact
        m_pending.erase( itr );
      else
        itr->nr_msgs = c->numberPendingMessages();
      
      if( !m_nr_msgs ) { // last msg read, switch icon
        Gtk::ImageLoader *il = Icons::IconForStatus( icqclient.getStatus(), icqclient.getInvisible() );
        m_pm.set( il->pix(), il->bit() );
      }
    }
  }
  update_applet_number();
  update_applet_tooltip();
}


void IckleApplet::update_applet_tooltip()
{
  ostringstream ostr;

  ostr << icqclient.getUIN() << " " << Status_text[icqclient.getStatus()] << endl;
  ostr << m_nr_online_users << " of " << m_nr_users << " users connected" << endl;
  if( m_nr_msgs ) {
    ostr << m_nr_msgs << ((m_nr_msgs > 1) ? " events" : " event")  << " pending:" << endl;
    for( list<msg_entry>::iterator itr = m_pending.begin();
         itr != m_pending.end(); ++itr ) {
      ostr << itr->alias << " (" << itr->nr_msgs << ")" << endl;
    }
  }
  applet_widget_set_tooltip( APPLET_WIDGET(m_applet), ostr.str().c_str() );
}


void IckleApplet::update_applet_number()
{
  ostringstream ostr;

  if( m_nr_msgs )
    ostr << m_nr_msgs;
  else
    ostr << m_nr_online_users;
  m_nr.set_text( ostr.str() );
}


IckleApplet::IckleApplet()
  : m_hbox(false, 0)
{
  m_applet = NULL;
  m_nr.set_text( "0" );

  m_nr_msgs = 0;
  m_nr_users = 0;
  m_nr_online_users = 0;
}


void IckleApplet::init(int argc, char* argv[], IckleGUI &g)
{
  Gtk::ImageLoader *il;

  m_gui = &g;

  // setup callbacks
  icqclient.statuschanged.connect( slot(this, &IckleApplet::icq_statuschanged_cb) );
  icqclient.messaged.connect(slot(this,&IckleApplet::icq_messaged_cb));
  icqclient.contactlist.connect(slot(this,&IckleApplet::icq_contactlist_cb));

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
  update_applet_tooltip();

  gtk_widget_show_all(m_applet);
}

void IckleApplet::quit()
{
  if( m_applet != NULL )
    applet_widget_remove( APPLET_WIDGET(m_applet) );
}
