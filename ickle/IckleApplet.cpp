/* $Id: IckleApplet.cpp,v 1.32 2003-01-02 16:39:55 barnabygray Exp $
 *
 * GNOME applet for ickle.
 *
 * Copyright (C) 2001-2002 Nils Nordman <nino@nforced.com>
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

#include <map>

#include <libicq2000/Client.h>
#include <libicq2000/constants.h>

#include <gnome.h>

#include "sstream_fix.h"
#include "main.h"
#include "IckleApplet.h"
#include "Icons.h"

using std::ostringstream;
using std::string;
using std::map;
using std::endl;

void IckleApplet::applet_click_converter(GtkWidget *, GdkEventButton *ev, gpointer data)
{
  ((IckleApplet *)data)->applet_click_cb(ev);
}


void IckleApplet::applet_sizechange_converter(GtkWidget *w, int size, gpointer data)
{
    ((IckleApplet *)data)->set_applet_size(size,
                                           applet_widget_get_panel_orient(APPLET_WIDGET(w) ) );
}
 

void IckleApplet::applet_orientchange_converter(AppletWidget *, PanelOrientType orient, gpointer data)
{
  ((IckleApplet *)data)->applet_orientchange_cb(orient);
}

void IckleApplet::applet_click_cb(GdkEventButton *ev)
{
  if (!ev || ev->button != 1 || ev->type != GDK_BUTTON_PRESS)
    return;
  
  if (!m_msg_queue.get_size() || ev->state & GDK_SHIFT_MASK)
    toggle_gui();
  else {
    MessageEvent *ev = m_msg_queue.get_first_message();
    if (ev->getServiceType() == MessageEvent::ICQ) {
      ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
      user_popup.emit(icq->getICQContact()->getUIN());
    }
  }
}


void IckleApplet::applet_status_menu_cb(AppletWidget *, gpointer data)
{
  ICQ2000::Status st = (ICQ2000::Status)(int)data;
  if (g_settings.getValueBool("status_classic_invisibility"))
    icqclient.setStatus(st, false);
  else
    icqclient.setStatus(st);
}


void IckleApplet::applet_toogle_menu_cb(AppletWidget *, gpointer data)
{
  ((IckleApplet *)data)->toggle_gui();
}


gint IckleApplet::applet_delete_cb(GtkWidget *, GdkEvent *, gpointer data)
{
  IckleApplet *appl = ((IckleApplet *)data);
  appl->m_applet = NULL;
  delete appl->m_box;
  if (appl->m_gui->is_realized())
    appl->exit.emit();
  return FALSE;
}


void IckleApplet::applet_orientchange_cb(PanelOrientType orient)
{
  int size = 48;

#ifdef HAVE_PANEL_PIXEL_SIZE
  size = applet_widget_get_panel_pixel_size(APPLET_WIDGET(m_applet));
#endif
  reset_applet_box();
  set_applet_size(size, orient);
}


void IckleApplet::queue_added_cb(MessageEvent*)
{
  if (!m_applet)
    return;
  
  update_applet_icon();
  update_applet_number();
  update_applet_tooltip();
}

void IckleApplet::queue_removed_cb(MessageEvent*)
{
  if (!m_applet)
    return;
  
  update_applet_icon();
  update_applet_number();
  update_applet_tooltip();
}

void IckleApplet::icq_self_status_change_cb(ICQ2000::StatusChangeEvent *)
{
  if (!m_applet) 
    return;

  if (!m_msg_queue.get_size()) // we don't even display the status for (m_msg_queue.get_size() > 0)
    update_applet_icon();

  update_applet_tooltip();
}

void IckleApplet::icq_status_change_cb(ICQ2000::StatusChangeEvent *ev)
{
  if (!m_applet) 
    return;

  ICQ2000::ContactRef c = ev->getContact();
  
  if(ev->getOldStatus() == ICQ2000::STATUS_OFFLINE)
    m_online_users.push_back(c->getUIN());
  else if (ev->getStatus() == ICQ2000::STATUS_OFFLINE)
    m_online_users.remove(c->getUIN());

  update_applet_number();
  update_applet_tooltip();
}

void IckleApplet::icq_contactlist_cb(ICQ2000::ContactListEvent *ev)
{
  if (!m_applet)
    return;

  ICQ2000::ContactRef c = ev->getContact();
  
  if (ev->getType() == ICQ2000::ContactListEvent::UserAdded) {
    ++m_nr_users;
  }
  else if (ev->getType() == ICQ2000::ContactListEvent::UserRemoved) {
    --m_nr_users;
    m_online_users.remove(c->getUIN());
    update_applet_icon();
  }
  update_applet_number();
  update_applet_tooltip();
}


void IckleApplet::icons_changed_cb()
{
  if (!m_applet)
    return;

  update_applet_icon();
}


void IckleApplet::set_applet_size(int size, PanelOrientType orient)
{
  int width, height;

  width = height = -1;

  if (orient == ORIENT_UP || orient == ORIENT_DOWN)
    height = size - (size / 8);
  else
    width = size - (size / 8);

  m_frame.set_usize(width, height);
}


void IckleApplet::update_applet_icon()
{
  Gtk::ImageLoader *il;

  if (!m_msg_queue.get_size())
    il = g_icons.IconForStatus(icqclient.getStatus(), icqclient.getInvisible());
  else {
    MessageEvent *ev = m_msg_queue.get_first_message();
    if (ev->getServiceType() == MessageEvent::ICQ) {
      ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);
      il = g_icons.IconForEvent(icq->getICQMessageType());
    }
  }

  m_pm.set(il->pix(), il->bit());
}


void IckleApplet::update_applet_tooltip()
{
  ostringstream ostr;
  map<string,bool> msgs;

  ostr << icqclient.getUIN() << " " << icqclient.getSelfContact()->getStatusStr() << endl;
  ostr << m_online_users.size() << " of " << m_nr_users << " users connected" << endl;
  if (m_msg_queue.get_size()) {
    ostr << m_msg_queue.get_size() << ((m_msg_queue.get_size() > 1) ? " events" : " event") << " pending:" << endl;

    for (MessageQueue::const_iterator itr = m_msg_queue.begin(); itr != m_msg_queue.end(); ++itr) {
      ICQMessageEvent *icqm = dynamic_cast<ICQMessageEvent *>(*itr);
      if (icqm && !msgs.count(icqm->getICQContact()->getAlias())) {
        ostr << icqm->getICQContact()->getAlias() << " (" <<
          m_msg_queue.get_contact_size(icqm->getICQContact()) << ")" << endl;
        msgs[icqm->getICQContact()->getAlias()] == true;
      }
    }
  }
  applet_widget_set_tooltip(APPLET_WIDGET(m_applet), ostr.str().c_str());
}


void IckleApplet::update_applet_number()
{
  ostringstream ostr;

  if (m_msg_queue.get_size())
    ostr << m_msg_queue.get_size();
  else
    ostr << m_online_users.size();

  m_nr.set_text(ostr.str());
}


void IckleApplet::reset_applet_box()
{
  int padding;
  PanelOrientType orient = applet_widget_get_panel_orient(APPLET_WIDGET(m_applet));

  if (m_box) { // already allocated, free this first
    m_box->remove(m_pm);
    m_box->remove(m_nr);
    m_frame.remove();
    delete m_box;
  }

  if (orient == ORIENT_UP || orient == ORIENT_DOWN) {
    m_box = new Gtk::HBox(false, 0);
    padding = 3;
  }
  else {
    m_box = new Gtk::VBox(false, 0);
    padding = 1;
  }

  m_box->pack_start(m_pm, true, true, padding);
  m_box->pack_end(m_nr, true, true, padding);
  m_box->show_all();
  m_frame.add(*m_box);
}


void IckleApplet::toggle_gui()
{
  if (m_gui->is_visible())
    m_gui->hide();
  else
    m_gui->show_all();
}


IckleApplet::IckleApplet(MessageQueue& mq)
  : m_msg_queue(mq)
{
  m_applet = NULL;
  m_box = NULL;
  m_nr.set_text("0");

  m_nr_users = 0;
}


void IckleApplet::init(int argc, char* argv[], IckleGUI &g)
{
  struct poptOption popts[] = {
    {NULL,'h', POPT_ARG_NONE, NULL, 0, NULL, NULL},
    {NULL,'b', POPT_ARG_STRING, NULL, 0, NULL, NULL},
    {NULL, 0, 0, NULL, 0, NULL, NULL}
  };
    
  m_gui = &g;

  // setup callbacks
  icqclient.self_contact_status_change_signal.connect(SigC::slot(*this, &IckleApplet::icq_self_status_change_cb));
  icqclient.contact_status_change_signal.connect(SigC::slot(*this, &IckleApplet::icq_status_change_cb));
  icqclient.contactlist.connect(SigC::slot(*this,&IckleApplet::icq_contactlist_cb));

  m_msg_queue.added.connect(SigC::slot(*this,&IckleApplet::queue_added_cb));
  m_msg_queue.removed.connect(SigC::slot(*this,&IckleApplet::queue_removed_cb));

  g_icons.icons_changed.connect(SigC::slot(*this, &IckleApplet::icons_changed_cb));

  // create applet
  applet_widget_init("ickle_applet", NULL, argc, argv, popts, 0, NULL);
  m_applet = applet_widget_new("ickle_applet");
  if (!m_applet)
    g_error("Can't create applet!\n");

  // connect applet signals
  gtk_widget_set_events(m_applet, gtk_widget_get_events(m_applet) | GDK_BUTTON_PRESS_MASK);
  gtk_widget_realize(m_applet);
  gtk_signal_connect(Gtk::OBJECT(m_applet), "button_press_event",
		     Gtk::SIGNAL_FUNC(applet_click_converter), this);
  gtk_signal_connect(Gtk::OBJECT(m_applet), "delete_event",
  		     Gtk::SIGNAL_FUNC(applet_delete_cb), this);
  gtk_signal_connect(Gtk::OBJECT(m_applet), "change_orient",
  		     Gtk::SIGNAL_FUNC(applet_orientchange_converter), this);

#ifdef HAVE_PANEL_PIXEL_SIZE
  gtk_signal_connect(Gtk::OBJECT(m_applet),"change_pixel_size",
		     Gtk::SIGNAL_FUNC(applet_sizechange_converter), this);
#endif

  // create context menus
  applet_widget_register_callback_dir(APPLET_WIDGET(m_applet), "status", _("Status"));
  applet_widget_register_callback(APPLET_WIDGET(m_applet), "status/online",
				  ICQ2000::Status_text[ICQ2000::STATUS_ONLINE],
				  applet_status_menu_cb, (void *)ICQ2000::STATUS_ONLINE);

  applet_widget_register_callback(APPLET_WIDGET(m_applet), "status/away",
				  ICQ2000::Status_text[ICQ2000::STATUS_AWAY],
				  applet_status_menu_cb, (void *)ICQ2000::STATUS_AWAY);

  applet_widget_register_callback(APPLET_WIDGET(m_applet), "status/na",
				  ICQ2000::Status_text[ICQ2000::STATUS_NA],
				  applet_status_menu_cb, (void *)ICQ2000::STATUS_NA);

  applet_widget_register_callback(APPLET_WIDGET(m_applet), "status/occupied",
				  ICQ2000::Status_text[ICQ2000::STATUS_OCCUPIED],
				  applet_status_menu_cb, (void *)ICQ2000::STATUS_OCCUPIED);

  applet_widget_register_callback(APPLET_WIDGET(m_applet), "status/dnd",
				  ICQ2000::Status_text[ICQ2000::STATUS_DND],
				  applet_status_menu_cb, (void *)ICQ2000::STATUS_DND);

  applet_widget_register_callback(APPLET_WIDGET(m_applet), "status/freeforchat",
				  ICQ2000::Status_text[ICQ2000::STATUS_FREEFORCHAT],
				  applet_status_menu_cb, (void *)ICQ2000::STATUS_FREEFORCHAT);

  applet_widget_register_callback(APPLET_WIDGET(m_applet), "status/offline",
				  ICQ2000::Status_text[ICQ2000::STATUS_OFFLINE],
				  applet_status_menu_cb, (void *)ICQ2000::STATUS_OFFLINE);

  applet_widget_register_callback(APPLET_WIDGET(m_applet), "toogle_gui",
				  "Show/hide main window", applet_toogle_menu_cb, this);

  // whip the layout together
  reset_applet_box();
  applet_widget_add(APPLET_WIDGET(m_applet), Gtk::WIDGET(m_frame.gtkobj()));

  update_applet_icon();
  update_applet_tooltip();

  gtk_widget_show_all(m_applet);
}

void IckleApplet::quit()
{
  if (m_applet != NULL)
    applet_widget_remove(APPLET_WIDGET(m_applet));
}
