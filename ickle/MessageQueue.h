/* $Id: MessageQueue.h,v 1.2 2002-03-30 14:48:09 nordman Exp $
 *
 * Queueing for MessageEvents.
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

#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include "MessageEvent.h"

#include <list>

#include <sigc++/signal_system.h>

// ============================================================================
//  MessageQueue
// ============================================================================

class MessageQueue
{
 private:
  typedef std::list<MessageEvent*>::iterator iterator;
  std::list<MessageEvent*> m_event_list;
  
 public:
  MessageQueue();
  ~MessageQueue();
  
  // global queue methods
  void add_to_queue(MessageEvent *ev);
  void remove_from_queue(MessageEvent *ev);
  MessageEvent* get_first_message();

  unsigned int get_size() const;
  bool empty() const;

  typedef std::list<MessageEvent*>::const_iterator const_iterator;
  const_iterator begin() const;
  const_iterator end() const;

  // per-contact methods
  MessageEvent* get_contact_first_message(const ICQ2000::ContactRef& c);
  unsigned int get_contact_size(const ICQ2000::ContactRef& c) const;
  void clear_queue_for_contact(const ICQ2000::ContactRef& c);
  
  // signals
  SigC::Signal1<void, MessageEvent*> added;
  SigC::Signal1<void, MessageEvent*> removed;
};

#endif

