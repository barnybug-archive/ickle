/* $Id: IckleApplet.h,v 1.7 2001-11-26 23:58:30 nordman Exp $
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

#ifndef ICKLEAPPLET_H
#define ICKLEAPPLET_H

#include <sigc++/signal_system.h>
#include <gtk--/box.h>
#include <gtk--/label.h> 
#include <gtk--/frame.h>
#include <gtk--/pixmap.h>
#include <applet-widget.h>
#include <list>

#include <Client.h>
#include "IckleGUI.h"

class IckleApplet : public SigC::Object {
  
 private:

  GtkWidget *   m_applet;
  Gtk::Frame    m_frame;
  Gtk::Box *    m_box;
  Gtk::Label    m_nr;
  Gtk::Pixmap   m_pm;
  IckleGUI *    m_gui;

  class msg_entry {
  public:
    msg_entry(Contact *c) : contact(c), nr_msgs(1) {}
    Contact *contact;
    int nr_msgs;
  };

  list<msg_entry> m_pending;            /* list of pending msg's */

  gint          m_nr_msgs;              /* nr of messages pending, != m_pending.size() */
  gint          m_nr_users;             /* nr of users on contactlist */
  gint          m_nr_online_users;      /* nr of online users on contactlist */

  // C-callback to member function converters
  static void   applet_click_converter          (GtkWidget *sender, GdkEventButton *ev, gpointer data);
  static void   applet_sizechange_converter     (GtkWidget *w, int size, gpointer data);
  static void   applet_orientchange_converter   (AppletWidget *applet, GNOME_Panel_OrientType orient, gpointer data);

  // callbacks
  void          applet_click_cb         (GdkEventButton *ev);
  static void   applet_status_menu_cb   (AppletWidget *applet, gpointer data);
  static void   applet_toogle_menu_cb   (AppletWidget *applet, gpointer data);
  static gint   applet_delete_cb        (GtkWidget *widget, GdkEvent  *event, gpointer data);
  void          applet_orientchange_cb  (PanelOrientType orient);
  bool          icq_messaged_cb         (MessageEvent *ev);
  void          icq_statuschanged_cb    (MyStatusChangeEvent *ev);
  void          icq_contactlist_cb      (ContactListEvent *ev);
  void          icons_changed_cb        ();
  
  // misc
  void          set_applet_size         (int size, PanelOrientType orient);
  void          update_applet_tooltip   ();
  void          update_applet_number    ();
  void          reset_applet_box        ();
  void          toggle_gui              ();

 public:

  IckleApplet();
  ~IckleApplet() {}

  void          init            (int argc, char* argv[], IckleGUI &gui);
  void          quit            ();
};

#endif // ICKLEAPPLET_H
