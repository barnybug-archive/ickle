/* $Id: EventSystem.cpp,v 1.6 2002-04-20 15:06:42 barnabygray Exp $
 *
 * EventSystem
 *
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>,
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

#include "EventSystem.h"

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "main.h"
#include "Settings.h"
#include "EventSubstituter.h"

using std::string;

using ICQ2000::ContactRef;

// ============================================================================
//  MessageQueue
// ============================================================================

EventSystem::EventSystem(MessageQueue& mq)
  : m_message_queue(mq), m_last_event_time (0.0)
{ }

void EventSystem::queue_added_cb(MessageEvent *ev)
{
  if (ev->getServiceType() != MessageEvent::ICQ) return;

  ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(ev);

  time_t t = ev->getTime();
  ContactRef c = icq->getICQContact();

  switch (icq->getICQMessageType()) {
  case ICQMessageEvent::Normal:
    event_system("event_message", c, t);
    break;
  case ICQMessageEvent::URL:
    event_system("event_url", c, t);
    break;
  case ICQMessageEvent::SMS:
    event_system("event_sms", c, t);
    break;
  case ICQMessageEvent::AuthReq:
  case ICQMessageEvent::AuthAck:
  case ICQMessageEvent::UserAdd:
    event_system("event_system", c, t);
    break;
  // what do we do with SMS_Receipt and EmailEx?
  }
}

void EventSystem::status_change_cb(ICQ2000::StatusChangeEvent *ev)
{
  if (
      // anything --> online
      (ev->getOldStatus() != ICQ2000::STATUS_ONLINE &&
       ev->getStatus() == ICQ2000::STATUS_ONLINE)
    ||
      // off/away/na --> dnd/occ/ffc
      ( (ev->getOldStatus() == ICQ2000::STATUS_OFFLINE ||
         ev->getOldStatus() == ICQ2000::STATUS_AWAY ||
         ev->getOldStatus() == ICQ2000::STATUS_NA)
       &&
        (ev->getStatus() == ICQ2000::STATUS_DND ||
         ev->getStatus() == ICQ2000::STATUS_OCCUPIED ||
         ev->getStatus() == ICQ2000::STATUS_FREEFORCHAT) )
     )
  {
    event_system("event_user_online", ev->getContact(), ev->getTime());
  }
}

void EventSystem::event_system(const string& s, const ContactRef& c, time_t t)
{
  if (!g_settings.getValueString(s).empty()) {
    timeval tv;
    gettimeofday (&tv, NULL);
    double current_time = double(tv.tv_sec) + (double(tv.tv_usec) * 1e-6);

    unsigned int threshold = g_settings.getValueUnsignedInt("event_repetition_threshold");
    bool repeated = (threshold != 0 && current_time < m_last_event_time + double(threshold)/1000);
        
    if ((!repeated) || g_settings.getValueBool("event_execute_all")) {
      EventSubstituter evs(m_message_queue, c);
      evs.set_event_time(t);
      evs.set_escape_shell(true);
      evs.set_repeated(repeated);
      evs << g_settings.getValueString(s) << " &";
      system(evs.str().c_str());

      m_last_event_time = current_time;
    }
  }
}
