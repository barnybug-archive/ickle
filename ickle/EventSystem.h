/* $Id: EventSystem.h,v 1.1 2002-03-28 18:29:02 barnabygray Exp $
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

#ifndef EVENTSYSTEM_H
#define EVENTSYSTEM_H

#include <string>

#include "MessageEvent.h"

// ============================================================================
//  EventSystem
// ============================================================================

class EventSystem : public SigC::Object
{
 private:
  void event_system(const std::string& s, const ICQ2000::ContactRef& c, time_t t);
  
 public:
  EventSystem();

  void queue_added_cb(MessageEvent *ev);
};

#endif

