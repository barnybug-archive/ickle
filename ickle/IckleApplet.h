/* $Id: IckleApplet.h,v 1.18 2002-03-28 21:51:49 barnabygray Exp $
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

#ifndef ICKLEAPPLET_H
#define ICKLEAPPLET_H

#include <sigc++/signal_system.h>
#include <gtk--/box.h>
#include <gtk--/label.h> 
#include <gtk--/frame.h>
#include <gtk--/pixmap.h>
#include <applet-widget.h>

#include <list>

#include "IckleGUI.h"

#include "MessageQueue.h"

class IckleApplet : public SigC::Object {
  
 private:
  MessageQueue& m_message_queue;

  GtkWidget *   m_applet;
  Gtk::Frame    m_frame;
  Gtk::Box *    m_box;
  Gtk::Label    m_nr;
  Gtk::Pixmap   m_pm;
  IckleGUI *    m_gui;

  gint          m_nr_msgs;              /* nr of messages pending, != m_pending.size() */
  gint          m_nr_users;             /* nr of users on contactlist */
  std::list<unsigned int> m_online_users; /* online users on contactlist */

  // C-callback to member function converters
  static void   applet_click_converter          (GtkWidget *sender, GdkEventButton *ev, gpointer data);
  static void   applet_sizechange_converter     (GtkWidget *w, int size, gpointer data);
  static void   applet_orientchange_converter   (AppletWidget *applet, GNOME_Panel_OrientType orient, gpointer data);

  // callbacks
  void          applet_click_cb          (GdkEventButton *ev);
  static void   applet_status_menu_cb    (AppletWidget *applet, gpointer data);
  static void   applet_toogle_menu_cb    (AppletWidget *applet, gpointer data);
  static gint   applet_delete_cb         (GtkWidget *widget, GdkEvent *event, gpointer data);
  void          applet_orientchange_cb   (PanelOrientType orient);
  void          icq_self_status_change_cb(ICQ2000::StatusChangeEvent *ev);
  void          icq_status_change_cb     (ICQ2000::StatusChangeEvent *ev);
  void          icq_contactlist_cb       (ICQ2000::ContactListEvent *ev);
  void          icons_changed_cb         ();
  
  void          queue_added_cb           (MessageEvent *ev);
  void          queue_removed_cb           (MessageEvent *ev);

  // misc
  void          set_applet_size         (int size, PanelOrientType orient);
  void          update_applet_icon      ();
  void          update_applet_tooltip   ();
  void          update_applet_number    ();
  void          reset_applet_box        ();
  void          toggle_gui              ();

 public:

  IckleApplet(MessageQueue& mq);
  ~IckleApplet() {}

  void          init            (int argc, char* argv[], IckleGUI &gui);
  void          quit            ();

  SigC::Signal1<void,unsigned int> user_popup;
  SigC::Signal0<void> exit;
};

#endif // ICKLEAPPLET_H
