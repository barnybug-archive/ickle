/* $Id: IdleTimer.h,v 1.1 2002-01-09 20:20:26 nordman Exp $
 *
 * IdleTimer: Used to implement idle-events for X.
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

#ifndef XIDLER_H
#define XIDLER_H

#include <time.h>
#include <sigc++/signal_system.h>

#ifdef USE_XSCREENSAVER
# include <X11/Xutil.h>
# include <X11/extensions/scrnsaver.h>
#endif

// Now fix header braindamage, "#define Status int", tsk tsk
#ifdef Status
# undef Status
#endif

class IdleTimer : public SigC::Object {
  
 private:

#ifdef USE_XSCREENSAVER
  XScreenSaverInfo *    m_xss_info;
#else
  int                   m_last_xpos;
  int                   m_last_ypos;
  time_t                m_last_timestamp;
#endif

  SigC::Connection      m_timeoutconn;  // connection for timeout callback
  ICQ2000::Status       m_prevstatus;   // previous status
  bool                  m_autostatus;   // whether the current status is set by us
  guint                 m_idletime;     // total idle time in seconds

  void                  get_idle                ();
  gint                  timer_cb                ();
  
 public:

  IdleTimer();
  ~IdleTimer();
};

#endif // XIDLER_H
