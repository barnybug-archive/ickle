/* $Id: IckleApplet.cpp,v 1.16 2001-12-26 23:24:19 barnabygray Exp $
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

#include <gnome.h>

#include <libicq2000/constants.h>

#include "sstream_fix.h"
#include "main.h"
#include "IckleApplet.h"
#include "Icons.h"

using std::ostringstream;

void IckleApplet::applet_click_converter(GtkWidget *sender, GdkEventButton *ev, gpointer data)
{
  ((IckleApplet *)data)->applet_click_cb(ev);
}


void IckleApplet::applet_sizechange_converter(GtkWidget *w, int size, gpointer data)
{
    ((IckleApplet *)data)->set_applet_size(size,
                                           applet_widget_get_panel_orient( APPLET_WIDGET(w) ) );
}
 

void IckleApplet::applet_orientchange_converter(AppletWidget *applet, PanelOrientType orient, gpointer data)
{
  ((IckleApplet *)data)->applet_orientchange_cb(orient);
}

void IckleApplet::applet_click_cb(GdkEventButton *ev)
{
  if (!ev || ev->button != 1 || ev->type != GDK_BUTTON_PRESS)
    return;
  
  if( !m_nr_msgs || ev->state & GDK_SHIFT_MASK )
    toggle_gui();
  else
    user_popup.emit( m_pending.front().contact->getUIN() );
}


void IckleApplet::applet_status_menu_cb(AppletWidget *applet, gpointer data)
{
  Status st = (Status)(int)data;
  icqclient.setStatus( st );
}


void IckleApplet::applet_toogle_menu_cb(AppletWidget *applet, gpointer data)
{
  ((IckleApplet *)data)->toggle_gui();
}


gint IckleApplet::applet_delete_cb(GtkWidget *widget, GdkEvent  *event, gpointer data)
{
  ((IckleApplet *)data)->m_applet = NULL;
  delete ((IckleApplet *)data)->m_box;
  return FALSE;
}


void IckleApplet::applet_orientchange_cb(PanelOrientType orient)
{
  int size = 48;

#ifdef HAVE_PANEL_PIXEL_SIZE
  size = applet_widget_get_panel_pixel_size( APPLET_WIDGET(m_applet) );
#endif
  reset_applet_box();
  set_applet_size( size, orient );
}


bool IckleApplet::icq_messaged_cb(MessageEvent* ev)
{
  Contact *c = ev->getContact();
  ostringstream ostr;
  
  if( !m_nr_msgs++ ) {
    Gtk::ImageLoader *il = g_icons.IconForEvent( ev->getType() );
    m_pm.set( il->pix(), il->bit() );
  }

  // record in msg list
  list<msg_entry>::iterator itr = m_pending.begin();
  for( ; itr->contact != c && itr != m_pending.end(); ++itr );
  if( itr == m_pending.end() )
    m_pending.push_back( msg_entry(c) );
  else
    ++itr->nr_msgs;
  
  update_applet_number();
  update_applet_tooltip();
  return false;
}


void IckleApplet::icq_statuschanged_cb(MyStatusChangeEvent *ev)
{
  Gtk::ImageLoader *il;

  if( !m_nr_msgs ) { // we don't even display the status for (m_nr_msgs > 0)
    il = g_icons.IconForStatus( ev->getStatus(), icqclient.getInvisible() );
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
    
    // find entry for this contact in msg list
    list<msg_entry>::iterator itr = m_pending.begin();
    for( ; itr->contact != c && itr != m_pending.end(); ++itr );

    if( itr != m_pending.end() ) {
      m_nr_msgs -= (itr->nr_msgs - c->numberPendingMessages());
      if( !c->numberPendingMessages() ) // no more pending messages from this contact
        m_pending.erase( itr );
      else
        itr->nr_msgs = c->numberPendingMessages();
      
      if( !m_nr_msgs ) { // last msg read, switch icon
        Gtk::ImageLoader *il = g_icons.IconForStatus( icqclient.getStatus(), icqclient.getInvisible() );
        m_pm.set( il->pix(), il->bit() );
      }
    }
  }
  update_applet_number();
  update_applet_tooltip();
}


void IckleApplet::icons_changed_cb()
{
  Gtk::ImageLoader *il;
  if( !m_nr_msgs )
    il = g_icons.IconForStatus( icqclient.getStatus(), icqclient.getInvisible() );
  else // we don't keep track of the exact event so just use normal
    il = g_icons.IconForEvent( MessageEvent::Normal );
  m_pm.set( il->pix(), il->bit() );
}


void IckleApplet::set_applet_size(int size, PanelOrientType orient)
{
  int width, height;

  width = height = -1;

  if( orient == ORIENT_UP || orient == ORIENT_DOWN ) {
    height = size - (size / 8);
  }
  else {
    width = size - (size / 8);
  }
  m_frame.set_usize( width, height );
}


void IckleApplet::update_applet_tooltip()
{
  ostringstream ostr;

  ostr << icqclient.getUIN() << " " << Status_text[icqclient.getStatus()] << endl;
  ostr << m_nr_online_users << " of " << m_nr_users << " users connected" << endl;
  if( m_nr_msgs ) {
    ostr << m_nr_msgs << ((m_nr_msgs > 1) ? " events" : " event")  << " pending:" << endl;
    for( list<msg_entry>::iterator itr = m_pending.begin(); itr != m_pending.end(); ++itr ) {
      ostr << itr->contact->getAlias() << " (" << itr->contact->numberPendingMessages() << ")" << endl;
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


void IckleApplet::reset_applet_box()
{
  int padding;
  PanelOrientType orient = applet_widget_get_panel_orient( APPLET_WIDGET(m_applet) );

  if( m_box ) { // already allocated, free this first
    m_box->remove(m_pm);
    m_box->remove(m_nr);
    m_frame.remove();
    delete m_box;
  }

  if( orient == ORIENT_UP || orient == ORIENT_DOWN ) {
    m_box = new Gtk::HBox(false, 0);
    padding = 3;
  }
  else {
    m_box = new Gtk::VBox(false, 0);
    padding = 1;
  }

  m_box->pack_start( m_pm, true, true, padding );
  m_box->pack_end( m_nr, true, true, padding );
  m_box->show_all();
  m_frame.add(*m_box);
}


void IckleApplet::toggle_gui()
{
  if( m_gui->is_visible() )
    m_gui->hide();
  else
    m_gui->show();
}


IckleApplet::IckleApplet()
{
  m_applet = NULL;
  m_box = NULL;
  m_nr.set_text( "0" );

  m_nr_msgs = 0;
  m_nr_users = 0;
  m_nr_online_users = 0;
}


void IckleApplet::init(int argc, char* argv[], IckleGUI &g)
{
  Gtk::ImageLoader *il;
  struct poptOption popts[] = {
    {NULL,'h', POPT_ARG_NONE, NULL, 0, NULL, NULL},
    {NULL,'b', POPT_ARG_STRING, NULL, 0, NULL, NULL},
    NULL,
  };
    
  m_gui = &g;

  // setup callbacks
  icqclient.statuschanged.connect( slot(this, &IckleApplet::icq_statuschanged_cb) );
  icqclient.messaged.connect(slot(this,&IckleApplet::icq_messaged_cb));
  icqclient.contactlist.connect(slot(this,&IckleApplet::icq_contactlist_cb));

  g_icons.icons_changed.connect( slot(this, &IckleApplet::icons_changed_cb) );

  // create applet
  applet_widget_init("ickle_applet", NULL, argc, argv, popts, 0, NULL);
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
  gtk_signal_connect(GTK_OBJECT(m_applet), "change_orient",
  		     GTK_SIGNAL_FUNC(applet_orientchange_converter), this);

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

  applet_widget_register_callback( APPLET_WIDGET(m_applet), "toogle_gui",
				   "Show/hide main window", applet_toogle_menu_cb, this);

  // whip the layout together
  il = g_icons.IconForStatus( STATUS_OFFLINE, false );
  m_pm.set( il->pix(), il->bit() );
  reset_applet_box();
  applet_widget_add(APPLET_WIDGET(m_applet), GTK_WIDGET(m_frame.gtkobj()) );
  update_applet_tooltip();

  gtk_widget_show_all(m_applet);
}

void IckleApplet::quit()
{
  if( m_applet != NULL )
    applet_widget_remove( APPLET_WIDGET(m_applet) );
}
