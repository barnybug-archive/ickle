/* $Id: IdleTimer.cpp,v 1.3 2002-01-25 14:25:54 nordman Exp $
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

#include <gtk--/main.h>
#include <algorithm>
#include <libicq2000/Client.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

#include "IdleTimer.h"
#include "main.h"
#include "Settings.h"

using std::max;
using std::min;

void IdleTimer::get_idle()
{
#ifdef USE_XSCREENSAVER
  
  int event_base, error_base;
  if( XScreenSaverQueryExtension(GDK_DISPLAY(), &event_base, &error_base) ) {
      XScreenSaverQueryInfo ( GDK_DISPLAY(), GDK_ROOT_WINDOW(), m_xss_info );
      m_idletime = m_xss_info->idle / 1000;
  }
  
#else // Check the cursor for movement
  
  Window w1, w2;
  int xpos, ypos, unused;
  
  if( m_last_timestamp == (time_t)0 )
    m_last_timestamp = time(NULL);

  XQueryPointer( GDK_DISPLAY(), GDK_ROOT_WINDOW(), &w1, &w2, &xpos, &ypos, &unused, &unused, (uint*)&unused );

  if( xpos != m_last_xpos || ypos != m_last_ypos ) { // moved cursor, reset idle time
    m_idletime = 0;
    m_last_xpos = xpos;
    m_last_ypos = ypos;
  }
  else
    m_idletime += time(NULL) - m_last_timestamp;

  m_last_timestamp = time(NULL);
  
#endif
}


gint IdleTimer::timer_cb()
{
  unsigned short auto_away = g_settings.getValueUnsignedShort("auto_away");
  unsigned short auto_na = g_settings.getValueUnsignedShort("auto_na");

  if( icqclient.getStatus() == ICQ2000::STATUS_OFFLINE || ( !auto_away && !auto_na ) )
    return 1;
  
  get_idle();

  // we don't make any assumptions about auto_away being smaller than auto_na,
  // if the user wants to set a smaller auto_na than auto_away, fine.

  unsigned short lim;
  if ( (lim = max(auto_away, auto_na)) * 60 > m_idletime )
    if ( (lim = min(auto_away, auto_na)) * 60 > m_idletime ) {
      if (m_autostatus) { // back from idling, reset previous status
        icqclient.setStatus( m_prevstatus );
        m_autostatus = false;
      }
      return 1;
    }

  ICQ2000::Status currstatus = icqclient.getStatus();
  
  if (!m_autostatus)
    m_prevstatus = currstatus;

  if ( lim == auto_na ) {
    if( currstatus != ICQ2000::STATUS_NA )
      icqclient.setStatus( ICQ2000::STATUS_NA, icqclient.getInvisible() );
  }
  else if( currstatus != ICQ2000::STATUS_AWAY )
    icqclient.setStatus( ICQ2000::STATUS_AWAY, icqclient.getInvisible() );

  m_autostatus = true;
  return 1;
}


IdleTimer::IdleTimer()
{
#ifdef USE_XSCREENSAVER
  m_xss_info = XScreenSaverAllocInfo();
#else
  m_last_xpos = m_last_ypos = -1;
  m_last_timestamp = (time_t)0;
#endif

  m_autostatus = false;
  m_idletime = 0;

  // setup callbacks
  m_timeoutconn = Gtk::Main::timeout.connect( slot( this, &IdleTimer::timer_cb ), 2000 );
}

IdleTimer::~IdleTimer()
{
  m_timeoutconn.disconnect();
}
